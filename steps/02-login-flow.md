# STEP 2: Login Flow — Every Single Line Explained

This file explains exactly what happens when you login to Sri Mart.
We follow the code from the browser button click to the final response.

---

## The Big Picture

```
You click "Login" button
        │
        ▼
[1] Browser collects your input (username, password)
        │
        ▼
[2] Browser sends HTTP POST request to server
        │
        ▼
[3] Server receives request in sri.cpp
        │
        ▼
[4] Server routes to AuthController::loginUser()
        │
        ▼
[5] AuthController validates the request
        │
        ▼
[6] AuthController calls AuthService::loginUser()
        │
        ▼
[7] AuthService finds the user by username
        │
        ▼
[8] AuthService hashes the entered password
        │
        ▼
[9] AuthService compares hashed password with stored hash
        │
        ▼
[10] AuthService generates an authentication token
        │
        ▼
[11] AuthService saves the token → user mapping
        │
        ▼
[12] AuthService returns token + user info
        │
        ▼
[13] AuthController sends JSON response back to browser
        │
        ▼
You see "Login successful!" and get a token
```

---

## STEP 1: The Login Form (login.html)

```html
<!-- This is the Login Form in login.html -->
<div id="loginForm" class="form active">
    <div class="form-group">
        <label>Username</label>
        <input type="text" id="loginUsername" placeholder="Enter username">
    </div>
    <div class="form-group">
        <label>Password</label>
        <input type="password" id="loginPassword" placeholder="Enter password">
    </div>
    <button onclick="login()">Login</button>
</div>
```

**Line-by-line explanation:**
- `<div id="loginForm" class="form active">` — This is the login form container. "active" means it's visible (the register form is hidden).
- `<input type="text" id="loginUsername">` — Text field for username
- `<input type="password" id="loginPassword">` — Password field (shows dots instead of characters)
- `<button onclick="login()">` — When clicked, calls the `login()` JavaScript function

---

## STEP 2: You Fill In the Form and Click Login

You type:
- Username: `admin`
- Password: `admin123`

You click the "Login" button.

---

## STEP 3: The JavaScript Function Runs (login.html)

```javascript
// This function runs when you click the Login button
async function login() {
    // STEP 3a: Get the values you typed in the form
    const username = document.getElementById('loginUsername').value;
    // username = "admin"

    const password = document.getElementById('loginPassword').value;
    // password = "admin123"

    // STEP 3b: Check if all fields are filled
    if (!username || !password) {
        showMessage('Please fill in all fields', 'error');
        return;  // Stop here if any field is empty
    }

    // STEP 3c: Send the data to the server
    try {
        const response = await fetch('http://localhost:8080/api/v1/auth/login', {
            method: 'POST',  // We are sending login credentials
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
            // Converts to: '{"username":"admin","password":"admin123"}'
        });

        // STEP 3d: Get the server's response
        const data = await response.json();
        // data might be: {token: "ea4da133...", user: {username: "admin", ...}}

        // STEP 3e: Show success or error message
        if (response.ok) {
            showMessage('Login successful! Welcome ' + data.user.username, 'success');
            // Show the token in a box
            document.getElementById('tokenBox').textContent = 'Token: ' + data.token;
            document.getElementById('tokenBox').style.display = 'block';
            // Save the token in browser storage (so we can use it later)
            localStorage.setItem('token', data.token);
            localStorage.setItem('user', JSON.stringify(data.user));
        } else {
            showMessage(data.error || 'Login failed', 'error');
        }
    } catch (error) {
        showMessage('Cannot connect to server. Is it running?', 'error');
    }
}
```

**Line-by-line explanation:**
- `async function login()` — This is the login function. It waits for the server to respond.
- `document.getElementById('loginUsername').value` — Grabs the username you typed
- `if (!username || !password)` — Checks if either field is empty
- `fetch('http://localhost:8080/api/v1/auth/login', {...})` — Sends HTTP request to server
- `method: 'POST'` — POST method for sending credentials
- `body: JSON.stringify({ username, password })` — Converts your data to JSON string
- `const response = await fetch(...)` — Waits for server to respond
- `const data = await response.json()` — Converts response back to JavaScript object
- `data.user.username` — Gets the username from the response (e.g., "admin")
- `data.token` — Gets the authentication token (e.g., "ea4da133...")
- `localStorage.setItem('token', data.token)` — Saves the token in browser storage
- `localStorage.setItem('user', JSON.stringify(data.user))` — Saves the user info too

---

## STEP 4: The HTTP Request Travels to the Server

```http
POST /api/v1/auth/login HTTP/1.1
Host: localhost:8080
Content-Type: application/json

{"username":"admin","password":"admin123"}
```

**What each line means:**
- `POST` — The HTTP method (sending data)
- `/api/v1/auth/login` — The URL path (the login endpoint)
- `Content-Type: application/json` — Sending JSON data
- `{"username":"admin","password":"admin123"}` — Your login credentials

---

## STEP 5: The Server Receives the Request (sri.cpp)

```cpp
// In sri.cpp, the auth routes are registered
AuthController::initRoutes();
```

This tells the server: "When someone sends a request to /api/v1/auth/login, 
send it to the AuthController."

---

## STEP 6: The Route Matching (AuthController.cc)

```cpp
// In AuthController.cc, the login route is registered
void AuthController::initRoutes()
{
    // POST /api/v1/auth/login -> logs in a user and returns a token
    app().registerHandler("/api/v1/auth/login",
        &AuthController::loginUser,
        {Post});
}
```

**Line-by-line explanation:**
- `app().registerHandler(...)` — Registers a URL handler
- `"/api/v1/auth/login"` — The URL to match
- `&AuthController::loginUser` — The function to call
- `{Post}` — Only match POST requests

---

## STEP 7: The Controller Handles the Request (AuthController.cc)

```cpp
// This function is called when someone sends POST /api/v1/auth/login
void AuthController::loginUser(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    // STEP 7a: Extract the JSON body from the request
    auto json = req->getJsonObject();

    // STEP 7b: Validate that username and password are present
    if (!json || !json->isMember("username") || !json->isMember("password"))
    {
        Json::Value error;
        error["error"] = "Username and password are required";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // STEP 7c: Extract username and password from JSON
    std::string username = (*json)["username"].asString();
    // username = "admin"

    std::string password = (*json)["password"].asString();
    // password = "admin123"

    // STEP 7d: Call the service to authenticate
    auto result = AuthService::loginUser(username, password);

    // STEP 7e: Check if the service returned an error
    if (result.isMember("error"))
    {
        auto resp = HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(k401Unauthorized);  // 401 = Unauthorized
        callback(resp);
    }
    else
    {
        // STEP 7f: Send back the token and user info
        auto resp = HttpResponse::newHttpJsonResponse(result);
        callback(resp);
    }
}
```

**Line-by-line explanation:**
- `req->getJsonObject()` — Extracts the JSON body from the request
- `(*json)["username"].asString()` — Gets the "username" field as a string
- `AuthService::loginUser(username, password)` — Calls the service to check credentials
- `result.isMember("error")` — Checks if the service returned an error
- `k401Unauthorized` — HTTP status 401 means "You're not authorized" (wrong credentials)
- If no error, send back the token and user info

---

## STEP 8: The Service Does the Business Logic (AuthService.cc)

```cpp
// This function logs in a user
Json::Value AuthService::loginUser(const std::string &username, const std::string &password)
{
    // STEP 8a: Lock the storage (thread safety)
    std::lock_guard<std::mutex> lock(getMutex());

    // STEP 8b: Hash the password that was entered
    std::string hashedPassword = hashPassword(password);
    // If password = "admin123", hashedPassword = some scrambled string

    // STEP 8c: Search for the user by username
    for (auto &user : getUserStorage())
    {
        // STEP 8d: Check if username matches AND password hash matches
        if (user.username == username && user.password == hashedPassword)
        {
            // STEP 8e: Generate an authentication token
            std::string token = generateToken();
            // token = "ea4da133ee6f00cdead2691b91fd8257" (32 hex characters)

            // STEP 8f: Save the token → user_id mapping
            getTokenStorage()[token] = user.id;
            // Now this token is linked to this user's ID

            // STEP 8g: Prepare the response
            Json::Value result;
            result["token"] = token;      // Include the token
            result["user"] = user.toJson(); // Include the user info (without password)
            return result;
        }
    }

    // STEP 8h: If no user found or password wrong, return error
    Json::Value error;
    error["error"] = "Invalid username or password";
    return error;
}
```

**Line-by-line explanation:**
- `std::lock_guard<std::mutex> lock(getMutex())` — Lock the storage for thread safety
- `hashPassword(password)` — Hash the entered password to compare with stored hash
- `for (auto &user : getUserStorage())` — Loop through all users
- `user.username == username` — Check if username matches
- `user.password == hashedPassword` — Check if password hash matches
- `generateToken()` — Create a random authentication token
- `getTokenStorage()[token] = user.id` — Link the token to the user
- `result["token"] = token` — Add the token to the response
- `result["user"] = user.toJson()` — Add the user info to the response
- `error["error"] = "Invalid username or password"` — Wrong credentials

---

## STEP 9: The Password Comparison (AuthService.cc)

The password comparison works like this:

```
What you typed:     "admin123"
                           │
                           ▼
Hashed version:     hashPassword("admin123") = "1!2#3$4%5&"
                                                   │
                                                   ▼
Stored hash:        user.password = "1!2#3$4%5&"
                                                   │
                                                   ▼
Are they equal?     YES → Login successful!
                    NO  → "Invalid username or password"
```

**Line-by-line explanation:**
- `hashPassword(password)` — Hash the password you entered
- `user.password` — The password hash stored in the user object
- `user.password == hashedPassword` — Compare the two hashes
- If they match, the password is correct (even though we never store the plain text)

---

## STEP 10: The Token Generation (AuthService.cc)

```cpp
// This function generates a random authentication token
std::string AuthService::generateToken()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; i++)  // Generate 32 hex characters
    {
        ss << std::hex << dis(gen);  // Add a random hex digit (0-9, a-f)
    }
    return ss.str();
}
```

**Line-by-line explanation:**
- `std::random_device rd` — Random number generator
- `std::mt19937 gen(rd())` — Mersenne Twister engine
- `std::uniform_int_distribution<> dis(0, 15)` — Random number 0-15
- `for (int i = 0; i < 32; i++)` — Loop 32 times
- `ss << std::hex << dis(gen)` — Add a random hex digit
- Example output: `ea4da133ee6f00cdead2691b91fd8257`

---

## STEP 11: The Token Storage (AuthService.cc)

```cpp
// This stores the mapping: token → user_id
std::map<std::string, std::string> &AuthService::getTokenStorage()
{
    static std::map<std::string, std::string> tokens;
    return tokens;
}
```

**What happens when you login:**
```cpp
// After generating the token:
getTokenStorage()[token] = user.id;
// This creates an entry like:
// "ea4da133ee6f00cdead2691b91fd8257" → "aa367098-8f08-4898-b722-..."
```

**Line-by-line explanation:**
- `static std::map<std::string, std::string> tokens` — A map that stores key-value pairs
- Key: the token string (e.g., "ea4da133...")
- Value: the user ID (e.g., "aa367098-...")
- `getTokenStorage()[token] = user.id` — Creates a new entry in the map

---

## STEP 12: The Response Goes Back to the Browser

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
    "token": "ea4da133ee6f00cdead2691b91fd8257",
    "user": {
        "id": "aa367098-8f08-4898-b722-045339aed30d",
        "username": "admin",
        "email": "admin@srimart.com",
        "role": "admin",
        "created_at": "2026-08-13 12:00:00",
        "updated_at": "2026-08-13 12:00:00"
    }
}
```

**What each line means:**
- `HTTP/1.1 200 OK` — Status 200 means "Success"
- `token` — The authentication token you'll use for future requests
- `user` — Your user information (without password)

---

## STEP 13: The Browser Shows Success and Saves Token

```javascript
const data = await response.json();
// data = {token: "ea4da133...", user: {username: "admin", ...}}

if (response.ok) {
    showMessage('Login successful! Welcome admin', 'success');
    // Show the token
    document.getElementById('tokenBox').textContent = 'Token: ' + data.token;
    document.getElementById('tokenBox').style.display = 'block';
    // Save in browser storage
    localStorage.setItem('token', data.token);
    localStorage.setItem('user', JSON.stringify(data.user));
}
```

You see:
- "Login successful! Welcome admin"
- A box showing your token: `Token: ea4da133ee6f00cdead2691b91fd8257`

---

## STEP 14: Using the Token for Future Requests

When you want to access protected endpoints, you send the token in the header:

```javascript
// Example: Validate your token
const token = localStorage.getItem('token');
const response = await fetch('http://localhost:8080/api/v1/auth/validate', {
    headers: {
        'Authorization': 'Bearer ' + token
        // Sends: Authorization: Bearer ea4da133ee6f00cdead2691b91fd8257
    }
});
```

**The server checks:**
1. Is there an Authorization header?
2. Does it start with "Bearer "?
3. Is the token valid (exists in token storage)?
4. If yes → return the user info
5. If no → return "Invalid or expired token"

---

## Summary: Login Flow

```
1. You type username and password in the form
2. You click "Login" button
3. JavaScript sends POST request to /api/v1/auth/login
4. Server receives the request in AuthController::loginUser()
5. AuthController validates the request has username and password
6. AuthController calls AuthService::loginUser()
7. AuthService locks the storage (thread safety)
8. AuthService hashes the entered password
9. AuthService searches for user by username
10. AuthService compares stored hash with entered hash
11. If match: AuthService generates a token
12. AuthService saves token → user_id mapping
13. AuthService returns token + user info
14. AuthController sends 200 OK response with JSON
15. Browser receives the response
16. Browser saves token in localStorage
17. Browser shows "Login successful!"
```

---

## What is a Token?

A token is like a **visitor badge** at a conference:

```
At the entrance (login):
  You show your ID (password)
  You get a badge (token)

At every booth (API request):
  You show your badge (token in header)
  The booth checks your badge
  If valid → serves you
  If invalid → sends you away
```

**Token format:** `Bearer <token>`
**Example:** `Authorization: Bearer ea4da133ee6f00cdead2691b91fd8257`
