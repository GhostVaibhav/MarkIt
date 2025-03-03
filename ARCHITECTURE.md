# Architecture

## v1 Application Plans

`MarkIt` is an offline-first application that has the basic and sole intention to store, retrieve, delete and mark/unmark any **TODOs**.

We have a profile-based system which helps in segregating different **TODOs** to different scenarios.

The login flow of the application is as follows -

0. Key files are checked (`state.dat`, `key.dat` and `data_*.dat`)
1. The login credentails are stored in the `key.dat` file
2. The current session is stored in the `state.dat` file
3. If `state.dat` file
   - Doesn't exist, display the login screen for taking the login credentials, goto **4**
   - Exist, goto **5**
4. If `key.dat` file
   - Doesn't exist, store the credentials with the hashed password, goto **5**
   - Exist, check the credentials
     - If wrong, goto **3 (Login Screen)**
     - If correct, goto **5**
5. If `data_(SHA256(username@passwordHash)).dat` file
   - Doesn't exist, render empty screen
   - Exist, then render the **TODOs**

There are certain context actions in the main menu, which are listed below -

1. Create a **TODO**
2. Logout
   - Go backs to the login screen, deleting the `state.dat` file

There are context actions for each **TODO** as well, which are listed below -

1. Mark/Unmark - Toggle between complete and incomplete state
2. Delete the **TODO**

## v2 Application Plans

- Connect the application to the internet.

  - [`getpantry.cloud`](https://getpantry.cloud) is the online storage used.
  - The following steps would be modified -
    1. Redirect the user to the website, so that he/she can fill in the email and check the captcha. Let him/her keep the Pantry name.
    2. Paste the generated Pantry ID in the application.
    3. Buckets are created per user ID. (Bucket name is the SHA256 hash of the username and password of the user with a salt)
    4. The data associated with that particular user would be uploaded to the site.
    5. A patch would be stored in the form of `patch_(SHA256(username@passwordHash)).dat` which will be computed from the last save which was uploaded to the cloud.
    6. Pantry ID can be updated by providing it through CLI with options like (`--api-key=`)

- Provide a CLI functionality for better access.

  - The following functionalities will be provided -
    - Display control
      1. `--name like "${expr}"`
      2. `--completed {true|false}`
      3. `--time after "${time}" (before "${time}")`
      4. `--date after "${date}" (before "${date}")`
      5. `--limit ${number}`
      6. `--only-title`
      7. `--only-desc`
    - Update control
      1. `--mark-as-complete`
      2. `--mark-as-incomplete`
      3. `--delete`
    - Creation control
      1. `--create "${name}" "${desc}"`
    - Display
      1. `--list`
    - Profile functionality
      1. `--login`
      2. `--logout`
    - Online functionality
      1. `--push`
      2. `--api-key ${key}`
