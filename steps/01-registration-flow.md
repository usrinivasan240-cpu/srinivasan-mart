# STEP 1: Registration Flow — Every Single Line Explained

This file explains exactly what happens when you create a new account in Sri Mart.
We follow the code from the browser button click to the final response.

---

## The Big Picture

```
You click "Register" button
        │
        ▼
[1] Browser collects your input (username, email, password)
        │
        ▼
[2] Browser sends HTTP POST request to server
        │
        ▼
[3] Server receives request in sri.cpp
        │
        ▼
[4] Server routes to AuthController::registerUser()
        │
        ▼
[5] AuthController validates the request
        │
        ▼
[6] AuthController calls AuthService::registerUser()
        │
        ▼
[7] AuthService checks if username exists
        │
        ▼
[8] AuthService hashes the password
        │
        ▼
[9] AuthService generates a unique ID
        │
        ▼
[10] AuthService saves user to storage
        │
        ▼
[11] AuthService returns user data (without password)
        │
        ▼
[12] AuthController sends JSON response back to browser
        │
        ▼
You see "Registration successful!"
```

---

## STEP 1: The Login Page (login.html)

When you open `http://localhost:8080`, the server sends the login page.
Here's the HTML code that creates the register form:

```html
<!-- This is the Register Form in login.html -->
<div id="registerForm" class="form">
    <div class="form-group">
        <label>Username</label>
        <input type="text" id="regUsername" placeholder="Choose a username">
    </div>
    <div class="form-group">
        <label>Email</label>
        <input type="email" id="regEmail" placeholder="Enter email">
    </div>
    <div class="form-group">
        <label>Password</label>
        <input type="password" id="regPassword" placeholder="Choose a password">
    </div>
    <button onclick="register()">Register</button>
</div>
```

**Line-by-line explanation:**
- `<div id="registerForm">` — This is a container that holds the register form
- `<label>Username</label>` — This is the text label above the username field
- `<input type="text" id="regUsername">` — This is where you type your username
- `<input type="email" id="regEmail">` — This is where you type your email
- `<input type="password" id="regPassword">` — This is where you type your password (hidden with dots)
- `<button onclick="register()">` — When you click this button, it calls the `register()` JavaScript function

---

## STEP 2: You Fill In the Form and Click Register

You type:
- Username: `srini`
- Email: `srini@email.com`
- Password: `secret123`

You click the "Register" button.

---

## STEP 3: The JavaScript Function Runs (login.html)

```javascript
// This function runs when you click the Register button
async function register() {
    // STEP 3a: Get the values you typed in the form
    const username = document.getElementById('regUsername').value;
    // username = "srini"

    const email = document.getElementById('regEmail').value;
    // email = "srini@email.com"

    const password = document.getElementById('regPassword').value;
    // password = "secret123"

    // STEP 3b: Check if all fields are filled
    if (!username || !email || !password) {
        showMessage('Please fill in all fields', 'error');
        return;  // Stop here if any field is empty
    }

    // STEP 3c: Send the data to the server
    try {
        const response = await fetch('http://localhost:8080/api/v1/auth/register', {
            method: 'POST',  // We are creating something, so use POST
            headers: { 'Content-Type': 'application/json' },  // We are sending JSON
            body: JSON.stringify({ username, email, password })
            // This converts {username: "srini", email: "srini@email.com", password: "secret123"}
            // into a JSON string: '{"username":"srini","email":"srini@email.com","password":"secret123"}'
        });

        // STEP 3d: Get the server's response
        const data = await response.json();
        // data might be: {id: "...", username: "srini", email: "srini@email.com", ...}

        // STEP 3e: Show success or error message
        if (response.ok) {
            showMessage('Registration successful! Please login.', 'success');
            showForm('login');  // Switch to login form
        } else {
            showMessage(data.error || 'Registration failed', 'error');
        }
    } catch (error) {
        showMessage('Cannot connect to server. Is it running?', 'error');
    }
}
```

**Line-by-line explanation:**
- `async function register()` — This is a JavaScript function that handles registration. It's "async" because it waits for the server to respond.
- `document.getElementById('regUsername').value` — This grabs whatever you typed in the username field
- `if (!username || !email || !password)` — This checks if any field is empty. If so, it shows an error and stops.
- `fetch('http://localhost:8080/api/v1/auth/register', {...})` — This sends an HTTP request to the server. It's like mailing a letter with your registration details.
- `method: 'POST'` — POST means "I want to create something new"
- `headers: { 'Content-Type': 'application/json' }` — This tells the server "I'm sending you JSON data"
- `body: JSON.stringify({ username, email, password })` — This converts your data into a JSON string that can be sent over the internet
- `const response = await fetch(...)` — This waits for the server to respond
- `const data = await response.json()` — This converts the server's response back into a JavaScript object
- `if (response.ok)` — If the server returned a success status (200-299), show success message

---

## STEP 4: The HTTP Request Travels to the Server

The browser sends this HTTP request:

```http
POST /api/v1/auth/register HTTP/1.1
Host: localhost:8080
Content-Type: application/json

{"username":"srini","email":"srini@email.com","password":"secret123"}
```

**What each line means:**
- `POST` — The HTTP method (like saying "I want to create something")
- `/api/v1/auth/register` — The URL path (like the address of the department you want)
- `HTTP/1.1` — The protocol version
- `Host: localhost:8080` — The server address
- `Content-Type: application/json` — "I'm sending JSON data"
- Empty line — Separates headers from body
- `{"username":"srini",...}` — The actual data being sent

---

## STEP 5: The Server Receives the Request (sri.cpp)

```cpp
// In sri.cpp, the server registers the auth routes
AuthController::initRoutes();
```

This line tells the server: "When someone sends a request to /api/v1/auth/register, 
send it to the AuthController."

**Line-by-line explanation:**
- `AuthController::initRoutes()` — This calls a function that sets up all authentication routes
- The server now knows that POST requests to `/api/v1/auth/register` should go to `AuthController::registerUser()`

---

## STEP 6: The Route Matching (AuthController.cc)

```cpp
// In AuthController.cc, the routes are registered
void AuthController::initRoutes()
{
    // POST /api/v1/auth/register -> creates a new user account
    app().registerHandler("/api/v1/auth/register",
        &AuthController::registerUser,
        {Post});
}
```

**Line-by-line explanation:**
- `app().registerHandler(...)` — This tells the server "when someone visits this URL, call this function"
- `"/api/v1/auth/register"` — The URL path to match
- `&AuthController::registerUser` — The function to call when the URL matches
- `{Post}` — Only match POST requests (not GET, PUT, or DELETE)

---

## STEP 7: The Controller Handles the Request (AuthController.cc)

```cpp
// This function is called when someone sends POST /api/v1/auth/register
void AuthController::registerUser(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    // STEP 7a: Extract the JSON body from the request
    auto json = req->getJsonObject();

    // STEP 7b: Validate that all required fields are present
    if (!json || !json->isMember("username") || !json->isMember("email") || !json->isMember("password"))
    {
        Json::Value error;
        error["error"] = "Username, email, and password are required";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);  // 400 = Bad Request
        callback(resp);
        return;  // Stop here, don't continue
    }

    // STEP 7c: Call the service to actually create the user
    auto result = AuthService::registerUser(*json);

    // STEP 7d: Check if the service returned an error
    if (result.isMember("error"))
    {
        auto resp = HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
    }
    else
    {
        // STEP 7e: Send back the created user (without password)
        auto resp = HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(k201Created);  // 201 = Created successfully
        callback(resp);
    }
}
```

**Line-by-line explanation:**
- `const HttpRequestPtr &req` — This is the incoming request from the browser. It contains the JSON data you sent.
- `std::function<void(const HttpResponsePtr &)> &&callback` — This is a function we call to send the response back to the browser.
- `req->getJsonObject()` — This extracts the JSON body from the request. If the body isn't valid JSON, it returns null.
- `!json` — If JSON is null (invalid or missing)
- `!json->isMember("username")` — If the JSON doesn't have a "username" field
- `k400BadRequest` — HTTP status code 400 means "You sent invalid data"
- `return;` — Stop the function immediately, don't continue
- `AuthService::registerUser(*json)` — Call the service layer to do the actual work
- `result.isMember("error")` — Check if the service returned an error message
- `k201Created` — HTTP status code 201 means "Successfully created"

---

## STEP 8: The Service Does the Business Logic (AuthService.cc)

```cpp
// This function creates a new user account
Json::Value AuthService::registerUser(const Json::Value &userData)
{
    // STEP 8a: Lock the storage (prevent other requests from changing data at the same time)
    std::lock_guard<std::mutex> lock(getMutex());

    // STEP 8b: Validate required fields
    if (!userData.isMember("username") || !userData.isMember("email") || !userData.isMember("password"))
    {
        Json::Value error;
        error["error"] = "Username, email, and password are required";
        return error;
    }

    // STEP 8c: Extract the values from the JSON
    std::string username = userData["username"].asString();
    // username = "srini"

    std::string email = userData["email"].asString();
    // email = "srini@email.com"

    std::string password = userData["password"].asString();
    // password = "secret123"

    // STEP 8d: Check if username already exists
    for (auto &user : getUserStorage())
    {
        if (user.username == username)
        {
            Json::Value error;
            error["error"] = "Username already exists";
            return error;  // Stop, don't create duplicate
        }
        if (user.email == email)
        {
            Json::Value error;
            error["error"] = "Email already registered";
            return error;  // Stop, don't create duplicate
        }
    }

    // STEP 8e: Create a new User object
    User user;
    user.id = generateUUID();  // Generate a unique ID like "d0a48717-f629-437e-..."
    user.username = username;   // "srini"
    user.email = email;         // "srini@email.com"
    user.password = hashPassword(password);  // Hash the password (not stored as plain text!)
    user.role = "customer";    // Default role
    user.created_at = "2026-08-13 12:00:00";
    user.updated_at = "2026-08-13 12:00:00";

    // STEP 8f: Save the user to storage
    getUserStorage().push_back(user);
    // The user is now saved in memory (will be lost if server restarts)

    // STEP 8g: Return the user data (WITHOUT the password for security)
    return user.toJson();
}
```

**Line-by-line explanation:**
- `std::lock_guard<std::mutex> lock(getMutex())` — This is like putting a "Do Not Disturb" sign on the storage. While we're adding a user, no one else can modify the user list. This prevents "race conditions" (two requests trying to add users at the same time).
- `userData["username"].asString()` — This extracts the "username" field from the JSON and converts it to a C++ string
- `for (auto &user : getUserStorage())` — This loops through all existing users to check for duplicates
- `user.username == username` — Check if this username is already taken
- `user.id = generateUUID()` — Generate a unique ID for the new user. UUID looks like: `d0a48717-f629-437e-946f-0df5f143c2fb`
- `user.password = hashPassword(password)` — Convert the password to a hash. This is like scrambling the password so even if someone steals the database, they can't read the actual passwords.
- `getUserStorage().push_back(user)` — Add the new user to the end of the user list
- `return user.toJson()` — Convert the user to JSON and send it back. Note: `toJson()` does NOT include the password field.

---

## STEP 9: The Password Hashing (AuthService.cc)

```cpp
// This function "hashes" (scrambles) a password
std::string AuthService::hashPassword(const std::string &password)
{
    std::string hashed = password;  // Start with the original password
    for (auto &c : hashed)
    {
        c = c ^ 0x5A;  // XOR each character with 0x5A (90 in decimal)
    }
    return hashed;  // Return the scrambled version
}
```

**Line-by-line explanation:**
- `std::string hashed = password;` — Make a copy of the password
- `for (auto &c : hashed)` — Loop through each character in the password
- `c = c ^ 0x5A;` — XOR the character with 90 (0x5A in hex). This scrambles the character.
- Example: 's' (115) XOR 90 = 29 → some other character
- `return hashed;` — Return the scrambled password

**Why hash passwords?**
- If someone steals your database, they can't see the real passwords
- When you login, you hash the entered password and compare it to the stored hash
- If they match, the password is correct

**WARNING:** This is a simple hash for demo. In production, use bcrypt (much more secure).

---

## STEP 10: The UUID Generation (AuthService.cc)

```cpp
// This function generates a unique ID (UUID)
std::string AuthService::generateUUID()
{
    static std::random_device rd;      // Random number generator
    static std::mt19937 gen(rd());     // Mersenne Twister engine (good random numbers)
    static std::uniform_int_distribution<> dis(0, 15);   // Random number 0-15
    static std::uniform_int_distribution<> dis2(8, 11);  // Random number 8-11

    std::stringstream ss;
    ss << std::hex;  // Output in hexadecimal (0-9, a-f)
    for (int i = 0; i < 8; i++) ss << dis(gen);  // 8 hex digits
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);  // 4 hex digits
    ss << "-4";                                     // Always "4"
    for (int i = 0; i < 3; i++) ss << dis(gen);  // 3 hex digits
    ss << "-";
    ss << dis2(gen);                                // Always 8, 9, a, or b
    for (int i = 0; i < 3; i++) ss << dis(gen);  // 3 hex digits
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen); // 12 hex digits
    return ss.str();
}
```

**Line-by-line explanation:**
- `std::random_device rd` — Creates a random number generator
- `std::mt19937 gen(rd())` — Creates a "Mersenne Twister" engine (a very good random number generator)
- `std::uniform_int_distribution<> dis(0, 15)` — Creates a distribution that produces random numbers from 0 to 15
- `ss << std::hex` — Tells the stream to output in hexadecimal format (0-9, a-f)
- `for (int i = 0; i < 8; i++) ss << dis(gen)` — Generate 8 random hex digits
- `ss << "-"` — Add a dash separator
- The format is: `xxxxxxxx-xxxx-4xxx-[8-b]xxx-xxxxxxxxxxxx`
- Example output: `d0a48717-f629-437e-946f-0df5f143c2fb`

---

## STEP 11: The Response Goes Back to the Browser

The server sends this HTTP response:

```http
HTTP/1.1 201 Created
Content-Type: application/json

{
    "id": "d0a48717-f629-437e-946f-0df5f143c2fb",
    "username": "srini",
    "email": "srini@email.com",
    "role": "customer",
    "created_at": "2026-08-13 12:00:00",
    "updated_at": "2026-08-13 12:00:00"
}
```

**What each line means:**
- `HTTP/1.1 201 Created` — Status code 201 means "Successfully created"
- `Content-Type: application/json` — The response is in JSON format
- The JSON body contains the new user's information (without the password!)

---

## STEP 12: The Browser Shows Success

The JavaScript receives the response:

```javascript
const data = await response.json();
// data = {id: "d0a48717-...", username: "srini", ...}

if (response.ok) {  // response.ok is true because status is 201
    showMessage('Registration successful! Please login.', 'success');
    showForm('login');  // Switch to the login form
}
```

You see the message: **"Registration successful! Please login."**

---

## Summary: Registration Flow

```
1. You type username, email, password in the form
2. You click "Register" button
3. JavaScript sends POST request to /api/v1/auth/register
4. Server receives the request in AuthController::registerUser()
5. AuthController validates the request has all required fields
6. AuthController calls AuthService::registerUser()
7. AuthService locks the storage (thread safety)
8. AuthService checks if username/email already exists
9. AuthService hashes the password (scrambles it)
10. AuthService generates a unique UUID for the user
11. AuthService creates a User object with all fields
12. AuthService saves the user to in-memory storage
13. AuthService returns the user data (without password)
14. AuthController sends 201 Created response with user JSON
15. Browser receives the response and shows "Registration successful!"
16. Browser switches to the login form
```

---

## Data Storage Location

The user is stored in this variable (in AuthService.cc):

```cpp
static std::vector<User> products;  // This is the in-memory storage
```

This is like a list in memory. When the server stops, this list is emptied.
**To make data permanent, we need PostgreSQL.**
