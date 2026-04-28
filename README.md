# learnchemistry-backend

C++ backend for **learnchemistry.in** using **Boost.Asio/Boost.Beast** for HTTP, **JWT** auth, and **MySQL** (Connector/C++ X DevAPI) for persistence.

> Current status: ✅ **Signup + Login working** end-to-end (UI → API → DB).

---

## Table of Contents
1. Overview
2. Architecture & Request Flow
3. Project Structure
4. Dependencies
5. Build & Run (Windows / Visual Studio)
6. Configuration (AppConfig)
7. MySQL Schema (Auth-only)
8. API Endpoints (Auth-only)
9. Debugging Guide (breakpoints)
10. Common Issues & Fixes
11. Roadmap (next)

---

## 1) Overview

This backend exposes REST endpoints for authentication and later will support:
- User dashboard (“My Courses”)
- Free purchase flow (to validate order/enrollment pipeline)
- Razorpay payment integration (later)
- Email verification (later)

---

## 2) Architecture & Request Flow

### Layers

- **server/**: Boost.Asio/Beast accept loop + per-connection sessions
- **routing/**: method + path router
- **controllers/**: HTTP boundary (parse/validate JSON, map to responses)
- **services/**: business logic (auth)
- **repositories/**: DB access only
- **db/**: MySQL session pool
- **security/**: password hashing + JWT sign/verify
- **utils/**: HTTP JSON responses

### Function-wise Request Flow (end-to-end)

For any incoming request:

1. `server/Session::doRead()`
2. `server/Session::onRead()`  **(first function where the HTTP request is available)**
3. `server/HttpServer::handleRequest(req)`
   - Handles CORS preflight `OPTIONS`
   - Normalizes target
4. `routing/Router::match(method, path)`
5. Route handler → Controller method
6. Controller → Service
7. Service → Repository
8. Repository → MySQL
9. Controller builds `http::response` via `utils::HttpResponse::json(...)`
10. `server/Session::send()` → `http::async_write(...)` sends response to UI

> Important: `Session::send()` must keep the response alive (e.g., stored in a `shared_ptr` member) until `async_write` completes, otherwise Beast can crash during write.

---

## 3) Project Structure

```
learnchemistry-backend/
├── CMakeLists.txt
├── main.cpp
├── config/
│   ├── AppConfig.h
│   └── AppConfig.cpp
├── server/
│   ├── HttpServer.h
│   ├── HttpServer.cpp
│   ├── Session.h
│   └── Session.cpp
├── routing/
│   ├── Route.h
│   ├── Router.h
│   └── Router.cpp
├── middleware/
│   ├── Middleware.h
│   ├── MiddlewareChain.h
│   ├── MiddlewareChain.cpp
│   ├── LoggingMiddleware.*
│   └── JwtAuthMiddleware.*
├── controllers/
│   ├── AuthController.h
│   └── AuthController.cpp
├── services/
│   ├── AuthService.h
│   └── AuthService.cpp
├── repositories/
│   ├── UserRepository.h
│   └── UserRepository.cpp
├── db/
│   ├── MySqlPool.h
│   └── MySqlPool.cpp
├── security/
│   ├── JwtService.h
│   ├── JwtService.cpp
│   ├── PasswordHasher.h
│   └── PasswordHasher.cpp
└── utils/
    ├── HttpResponse.h
    └── HttpResponse.cpp
```

---

## 4) Dependencies

### Build tools
- CMake >= 3.20
- Visual Studio 2022 (MSVC)

### Libraries (as per your current CMake)
- Boost: `system`, `thread`
- nlohmann_json
- OpenSSL (Crypto)
- libsodium
- MySQL Connector/C++ (X DevAPI)

---

## 5) Build & Run (Windows / Visual Studio)

### 5.1 Configure (CMake)

If you use vcpkg toolchain:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>\scripts\buildsystems\vcpkg.cmake
```

### 5.2 Build

```powershell
cmake --build build --config Debug
```

### 5.3 Run

Run from Visual Studio (Local Windows Debugger) or:

```powershell
.\build\Debug\learnChemistry.exe
```

Default API host/port:
- `http://0.0.0.0:8080`

---

## 6) Configuration (AppConfig)

Current `config/AppConfig.h` fields (important ones):

- HTTP:
  - `host` (default `0.0.0.0`)
  - `port` (default `8080`)
- JWT:
  - `jwt_secret`
- MySQL (X DevAPI):
  - `db_host` (default `127.0.0.1`)
  - `db_port` (default `33060`)
  - `db_user`
  - `db_password`
  - `db_name` (default `learnchemistry`)
  - `db_pool_size`

### Security note (important)
**Do NOT commit DB passwords to source control**. Prefer environment variables or a local-only config file.

### Special characters in DB password
If your DB password contains special characters like `@`, do not build a `mysqlx://...` URI manually. Use `mysqlx::SessionSettings` (HOST/PORT/USER/PWD) to avoid URI parsing issues.

---

## 7) MySQL Schema (Auth-only)

For **signup/login only**, this is sufficient:

```sql
CREATE DATABASE IF NOT EXISTS learnchemistry
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_0900_ai_ci;

USE learnchemistry;

CREATE TABLE IF NOT EXISTS users (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  email VARCHAR(255) NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  role ENUM('USER','ADMIN') NOT NULL DEFAULT 'USER',
  email_verified TINYINT(1) NOT NULL DEFAULT 0,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

  PRIMARY KEY (id),
  UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

## 8) API Endpoints (Auth-only)

### 8.1 Signup
- **POST** `/v1/auth/signup`
- Body:
  ```json
  { "email": "abc@test.com", "password": "Pass@123" }
  ```
- Success: **201 Created**
  ```json
  { "message": "User registered successfully", "userId": 1, "token": "..." }
  ```
- Errors:
  - **409 Conflict** `{ "error": "User already exists" }`
  - **400 Bad Request** `{ "error": "missing email/password" }`

### 8.2 Login
- **POST** `/v1/auth/login`
- Body:
  ```json
  { "email": "abc@test.com", "password": "Pass@123" }
  ```
- Success: **200 OK**
  ```json
  { "userId": 1, "token": "..." }
  ```
- Error: **401 Unauthorized** `{ "error": "Invalid credentials" }`

### 8.3 CORS (development)
If UI runs on `http://localhost:5500` and API on `http://localhost:8080`, browsers send **OPTIONS** preflight.
Backend handles this inside `HttpServer::handleRequest()` and returns 204 with `Access-Control-Allow-*` headers.

---

## 9) Debugging Guide (breakpoints)

### 9.1 Network entry
- `server/Session.cpp` → `Session::onRead()`
  - For browser calls you will typically see `OPTIONS` first, then `POST`.

### 9.2 Routing
- `server/HttpServer.cpp` → `HttpServer::handleRequest()`
- `routing/Router.cpp` → `Router::match()`

### 9.3 Auth flow
- `controllers/AuthController.cpp` → `AuthController::signup()` / `login()`
- `services/AuthService.cpp` → `AuthService::signup()` / `login()`
- `repositories/UserRepository.cpp` → `findByEmail()` / `createUser()`

---

## 10) Common Issues & Fixes

### A) OPTIONS route mismatch (CORS)
Symptom: router sees `OPTIONS`, controller not called.
Fix: handle `OPTIONS` before router match and add CORS headers to all responses.

### B) Crash in `ioc_.run()` / Beast write stack
Cause: response object destroyed before `async_write` completes.
Fix: keep response in `Session` as a `shared_ptr` member until write callback.

### C) `invalid UTF-8` errors from nlohmann/json
Cause: raw binary included in JSON (e.g., HMAC bytes).
Fix:
- encode binary to hex/base64
- `HttpResponse::json()` uses `error_handler_t::replace` to prevent crashes.

### D) mysqlx Session creation fails with password containing '@'
Cause: URI parsing splits at '@'.
Fix: use `mysqlx::SessionSettings` instead of a URI string.

---

## 11) Roadmap (next)

- Protected endpoints with JWT middleware (`/v1/me`, `/v1/me/courses`)
- Course catalog (`/v1/courses`)
- Free purchase MVP (`/v1/orders/free`)
- Razorpay later
- Email verification later

---

## 12) Your current CMake and AppConfig (reference)

### Current CMake MySQL linking
You are using vcpkg imported target:

```cmake
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
...
target_link_libraries(learnChemistry PRIVATE unofficial::mysql-connector-cpp::connector)
```

### Current AppConfig DB defaults
- Host: `127.0.0.1`
- Port: `33060`
- DB: `learnchemistry`
- Pool: `4`
