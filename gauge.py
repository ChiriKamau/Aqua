# gauge.py
import os
import io
import firebase_admin
from firebase_admin import credentials, auth, db, storage
import datetime
from passlib.hash import scrypt
# Replace with your Firebase project credentials
cred = credentials.Certificate('firebase-adminsdk.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://espcam-69f58-default-rtdb.firebaseio.com',
    'storageBucket': 'espcam-69f58.appspot.com'
})

def login(email, password):
    try:
        user = auth.get_user_by_email(email)
        if user:
            auth_info = auth.get_user(user.uid)
            if auth_info.email_verified:
                token = auth.create_custom_token(user.uid)
                return token
            else:
                return None  # User email not verified
    except Exception as e:
        print(f"Error: {e}")
        return None

def get_all_data(email):
    try:
        user = auth.get_user_by_email(email)
        uid = user.uid
    except Exception as e:
        print(f"Error: {e}")
        return None, None, None, None, None, None

    ref = db.reference(f'UsersData/{uid}/readings')
    all_readings = ref.get()

    timestamps = []
    temperatures = []
    soilmoistures = []
    humidities = []
    uvs = []
    luminous = []

    if all_readings:
        for key, value in all_readings.items():
            timestamp_str = key  # Assuming the key is a string in the format '2024-4-17--12:14:52'
            try:
                timestamp = datetime.datetime.strptime(timestamp_str, '%Y-%m-%d--%H:%M:%S')
            except ValueError:
                continue  # Skip invalid timestamps

            temperature = value.get('temperature', 'N/A')
            Soilmoisture = value.get('SoilMoisture1', 'N/A')
            humidity = value.get('humidity', 'N/A')
            uv = value.get('UV', 'N/A')
            Luminous = value.get('Luminous', 'N/A')

            if temperature != 'N/A':
                timestamps.append(timestamp.strftime('%Y-%m-%d %H:%M:%S'))
                temperatures.append(temperature)
                soilmoistures.append(Soilmoisture)
                humidities.append(humidity)
                uvs.append(uv)
                luminous.append(Luminous)
    else:
        print("No readings found.")

    return timestamps, temperatures, soilmoistures, humidities, uvs, luminous

def get_topmost_data(email):
    try:
        user = auth.get_user_by_email(email)
        uid = user.uid
    except Exception as e:
        print(f"Error: {e}")
        return None

    ref = db.reference(f'UsersData/{uid}/readings')
    # Get only the last reading
    last_reading = ref.order_by_key().limit_to_last(1).get()

    if last_reading:
        last_reading_id = list(last_reading.keys())[0]
        topmost_data = last_reading[last_reading_id]

        # Create a dictionary with the required data
        gauge_data = {
            "soilm1": topmost_data.get("SoilMoisture1"),
            "soilm2": topmost_data.get("SoilMoisture2"),
            "Temperature": topmost_data.get("temperature"),
            "Humidity": topmost_data.get("humidity"),
            "Luminus": topmost_data.get("Luminous"),
            "UV": topmost_data.get("UV"),
            "ATM": topmost_data.get("ATM"),
            "chart_data": []  # No chart data as we're only retrieving the last reading
        }
        return gauge_data
    else:
        return None

# Function to get images from Firebase Storage
def get_images_from_firebase():
    bucket = storage.bucket()
    blob_list = bucket.list_blobs(prefix='images/')
    image_urls = []
    for blob in blob_list:
        if blob.name.endswith(('.jpg', '.png', '.gif')):
            image_urls.append(blob.generate_signed_url(datetime.timedelta(seconds=300), method='GET'))
    return image_urls

def hash_password(password):
    # Use the scrypt hash function from passlib
    hashed_password = scrypt.hash(password)
    return hashed_password

def signup_user(email, password):
    try:
        hashed_password = hash_password(password)
        user = auth.create_user(
            email=email,
            password=hashed_password
        )
        return {'success': True, 'user_id': user.uid}
    except Exception as e:
        return {'success': False, 'error': str(e)}