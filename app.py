# app.py
from flask import Flask, render_template, request, redirect, url_for, session, jsonify
from gauge import login, get_topmost_data, get_all_data, get_images_from_firebase
from gauge import signup_user
app = Flask(__name__)
app.secret_key = 'your_secret_key'


@app.route('/')
def landing():
    return render_template('landing.html')

@app.route('/signup')
def signup():
    return render_template('signup.html')

@app.route('/signup', methods=['POST'])
def signup_form():
    email = request.json.get('email')
    password = request.json.get('password')
    confirm_password = request.json.get('confirm_password')

     # Perform validations
    if not email or not password or not confirm_password:
        return jsonify({'error': 'Please fill in all fields.'}), 400
    elif password != confirm_password:
        return jsonify({'error': 'Passwords do not match.'}), 400
    elif len(password) < 8:
        return jsonify({'error': 'Password should be at least 8 characters long.'}), 400



    result = signup_user(email, password)
    if result['success']:
        return redirect(url_for('login_page'))
    else:
        return jsonify(result), 400
    


@app.route('/login', methods=['GET', 'POST'])
def login_page():
    if request.method == 'POST':
        email = request.form['email']
        password = request.form['password']
        token = login(email, password)
        if token:
            session['token'] = token
            session['email'] = email  # Store the email in the session
            return redirect(url_for('dashboard'))
        else:
            error = 'Invalid email or password'
            return render_template('login.html', error=error)
    return render_template('login.html')

   
@app.route('/graph')
def graph():
  return render_template('graph.html')

@app.route('/dashboard')
def dashboard():
    if 'token' in session:
        email = session.get('email')  # Assuming you store the email in the session
        data = get_topmost_data(email)
        if data is not None:
            json_data = jsonify(data).get_data(as_text=True)
            return render_template('dashboard.html')
        else:
            return "No data available"
    else:
        return redirect(url_for('login_page'))

 


if __name__ == '__main__':
    app.run(debug=True)