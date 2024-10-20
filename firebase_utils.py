import firebase_admin
from firebase_admin import auth, credentials,db
import datetime
from passlib.hash import scrypt

cred = credentials.Certificate('firebase-adminsdk.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://espcam-69f58-default-rtdb.firebaseio.com'
})

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

