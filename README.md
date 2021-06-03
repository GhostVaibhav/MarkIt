<h1 align="center">
<b> Todo </b>
</h1>
<h4 align="center">
<b> A cross-platform TODO app in teminal with simple interface and lightning fast speed for everyone </b>
</h4>
&nbsp;

## **Dependencies**
### This project uses:
* [libcurl](https://github.com/curl/curl)
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [PDCurses](https://github.com/wmcbrine/PDCurses) for Windows
* ncurses for UNIX-based system
## **Building the project**
### **On Windows** <svg role="img" fill="#0078D6" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Windows</title><path d="M0 3.449L9.75 2.1v9.451H0m10.949-9.602L24 0v11.4H10.949M0 12.6h9.75v9.451L0 20.699M10.949 12.6H24V24l-12.9-1.801"/></svg>
You can simply use the libraries and the header files which come preinstalled in the ```include``` and the ```lib``` folder. However, if you wish to build them yourself, you can have a look at the repository's page mentioned above and build them on your own.
> Note: All the libraries which come preinstalled are compiled as **x86** for maximal compatibility.
### **On UNIX-based systems** <svg role="img" fill="#000000" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>macOS</title><path d="M0 14.727h.941v-2.453c0-.484.318-.835.771-.835.439 0 .71.276.71.722v2.566h.915V12.25c0-.48.31-.812.764-.812.46 0 .718.28.718.77v2.518h.94v-2.748c0-.801-.517-1.334-1.307-1.334-.578 0-1.054.31-1.247.805h-.023c-.147-.514-.552-.805-1.118-.805-.545 0-.968.306-1.142.771H.903v-.695H0v4.006zm7.82-.646c-.408 0-.68-.208-.68-.537 0-.318.26-.522.714-.552l.926-.057v.307c0 .483-.427.839-.96.839zm-.284.71c.514 0 1.017-.268 1.248-.703h.018v.639h.908v-2.76c0-.804-.647-1.33-1.64-1.33-1.021 0-1.66.537-1.701 1.285h.873c.06-.332.344-.548.79-.548.464 0 .748.242.748.662v.287l-1.058.06c-.976.061-1.524.488-1.524 1.199 0 .721.564 1.209 1.338 1.209zm6.305-2.642c-.065-.843-.719-1.512-1.777-1.512-1.164 0-1.92.805-1.92 2.087 0 1.3.756 2.082 1.928 2.082 1.005 0 1.697-.59 1.772-1.485h-.888c-.087.453-.397.725-.873.725-.597 0-.982-.483-.982-1.322 0-.824.381-1.323.975-1.323.502 0 .8.321.876.748h.889zm2.906-2.967c-1.591 0-2.589 1.085-2.589 2.82 0 1.735.998 2.816 2.59 2.816 1.586 0 2.584-1.081 2.584-2.816 0-1.735-.997-2.82-2.585-2.82zm0 .832c.971 0 1.591.77 1.591 1.988 0 1.213-.62 1.984-1.59 1.984-.976 0-1.592-.77-1.592-1.984 0-1.217.616-1.988 1.591-1.988zm2.982 3.178c.042 1.006.866 1.626 2.12 1.626 1.32 0 2.151-.65 2.151-1.686 0-.813-.469-1.27-1.576-1.523l-.627-.144c-.67-.158-.945-.37-.945-.733 0-.453.415-.756 1.032-.756.623 0 1.05.306 1.096.817h.93c-.023-.96-.817-1.61-2.019-1.61-1.187 0-2.03.653-2.03 1.62 0 .78.477 1.263 1.482 1.494l.707.166c.688.163.967.39.967.782 0 .454-.457.779-1.115.779-.665 0-1.167-.329-1.228-.832h-.945z"/></svg> <svg role="img" fill="#E95420" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Ubuntu</title><path d="M12 0c6.623 0 12 5.377 12 12s-5.377 12-12 12S0 18.623 0 12 5.377 0 12 0zm3.279 17.68c-.766.441-1.029 1.422-.586 2.189.441.765 1.422 1.028 2.188.584.766-.441 1.029-1.422.585-2.189-.441-.765-1.421-1.028-2.187-.584zm-3.279-1c-.705 0-1.373-.157-1.971-.435L8.916 18.24c.93.459 1.978.721 3.084.721.646 0 1.268-.091 1.86-.256.104-.643.485-1.234 1.095-1.587.609-.351 1.313-.386 1.92-.156 1.186-1.163 1.957-2.749 2.07-4.515l-2.285-.033c-.21 2.391-2.215 4.266-4.66 4.266zM7.32 12c0-1.583.787-2.981 1.99-3.83L8.14 6.209c-1.404.93-2.445 2.369-2.881 4.035.506.404.83 1.034.83 1.74 0 .704-.324 1.319-.83 1.739.436 1.665 1.477 3.104 2.881 4.034l1.17-1.965C8.107 14.97 7.32 13.574 7.32 12zm-3.48-1.602c-.885 0-1.602.717-1.602 1.602s.717 1.602 1.602 1.602S5.441 12.885 5.441 12s-.716-1.602-1.601-1.602zM12 7.32c2.445 0 4.45 1.875 4.66 4.265l2.285-.034c-.113-1.765-.885-3.35-2.07-4.516-.609.232-1.313.194-1.92-.154-.609-.352-.99-.945-1.095-1.591-.594-.16-1.214-.25-1.86-.25-1.11 0-2.155.26-3.084.72l1.113 1.995c.6-.279 1.268-.435 1.971-.435zm3.279-1.001c.765.442 1.746.181 2.189-.585.441-.765.181-1.746-.588-2.19-.765-.44-1.746-.179-2.189.589-.441.764-.18 1.744.588 2.186z"/></svg> <svg role="img" fill="#35BF5C" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Manjaro</title><path d="M0 0v24h6.75V6.75h8.625V0H0zm8.625 8.625V24h6.75V8.625h-6.75zM17.25 0v24H24V0h-6.75z"/></svg> <svg role="img" fill="#A81D33" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Debian</title><path d="M13.88 12.685c-.4 0 .08.2.601.28.14-.1.27-.22.39-.33a3.001 3.001 0 01-.99.05m2.14-.53c.23-.33.4-.69.47-1.06-.06.27-.2.5-.33.73-.75.47-.07-.27 0-.56-.8 1.01-.11.6-.14.89m.781-2.05c.05-.721-.14-.501-.2-.221.07.04.13.5.2.22M12.38.31c.2.04.45.07.42.12.23-.05.28-.1-.43-.12m.43.12l-.15.03.14-.01V.43m6.633 9.944c.02.64-.2.95-.38 1.5l-.35.181c-.28.54.03.35-.17.78-.44.39-1.34 1.22-1.62 1.301-.201 0 .14-.25.19-.34-.591.4-.481.6-1.371.85l-.03-.06c-2.221 1.04-5.303-1.02-5.253-3.842-.03.17-.07.13-.12.2a3.551 3.552 0 012.001-3.501 3.361 3.362 0 013.732.48 3.341 3.342 0 00-2.721-1.3c-1.18.01-2.281.76-2.651 1.57-.6.38-.67 1.47-.93 1.661-.361 2.601.66 3.722 2.38 5.042.27.19.08.21.12.35a4.702 4.702 0 01-1.53-1.16c.23.33.47.66.8.91-.55-.18-1.27-1.3-1.48-1.35.93 1.66 3.78 2.921 5.261 2.3a6.203 6.203 0 01-2.33-.28c-.33-.16-.77-.51-.7-.57a5.802 5.803 0 005.902-.84c.44-.35.93-.94 1.07-.95-.2.32.04.16-.12.44.44-.72-.2-.3.46-1.24l.24.33c-.09-.6.74-1.321.66-2.262.19-.3.2.3 0 .97.29-.74.08-.85.15-1.46.08.2.18.42.23.63-.18-.7.2-1.2.28-1.6-.09-.05-.28.3-.32-.53 0-.37.1-.2.14-.28-.08-.05-.26-.32-.38-.861.08-.13.22.33.34.34-.08-.42-.2-.75-.2-1.08-.34-.68-.12.1-.4-.3-.34-1.091.3-.25.34-.74.54.77.84 1.96.981 2.46-.1-.6-.28-1.2-.49-1.76.16.07-.26-1.241.21-.37A7.823 7.824 0 0017.702 1.6c.18.17.42.39.33.42-.75-.45-.62-.48-.73-.67-.61-.25-.65.02-1.06 0C15.082.73 14.862.8 13.8.4l.05.23c-.77-.25-.9.1-1.73 0-.05-.04.27-.14.53-.18-.741.1-.701-.14-1.431.03.17-.13.36-.21.55-.32-.6.04-1.44.35-1.18.07C9.6.68 7.847 1.3 6.867 2.22L6.838 2c-.45.54-1.96 1.611-2.08 2.311l-.131.03c-.23.4-.38.85-.57 1.261-.3.52-.45.2-.4.28-.6 1.22-.9 2.251-1.16 3.102.18.27 0 1.65.07 2.76-.3 5.463 3.84 10.776 8.363 12.006.67.23 1.65.23 2.49.25-.99-.28-1.12-.15-2.08-.49-.7-.32-.85-.7-1.34-1.13l.2.35c-.971-.34-.57-.42-1.361-.67l.21-.27c-.31-.03-.83-.53-.97-.81l-.34.01c-.41-.501-.63-.871-.61-1.161l-.111.2c-.13-.21-1.52-1.901-.8-1.511-.13-.12-.31-.2-.5-.55l.14-.17c-.35-.44-.64-1.02-.62-1.2.2.24.32.3.45.33-.88-2.172-.93-.12-1.601-2.202l.15-.02c-.1-.16-.18-.34-.26-.51l.06-.6c-.63-.74-.18-3.102-.09-4.402.07-.54.53-1.1.88-1.981l-.21-.04c.4-.71 2.341-2.872 3.241-2.761.43-.55-.09 0-.18-.14.96-.991 1.26-.7 1.901-.88.7-.401-.6.16-.27-.151 1.2-.3.85-.7 2.421-.85.16.1-.39.14-.52.26 1-.49 3.151-.37 4.562.27 1.63.77 3.461 3.011 3.531 5.132l.08.02c-.04.85.13 1.821-.17 2.711l.2-.42M9.54 13.236l-.05.28c.26.35.47.73.8 1.01-.24-.47-.42-.66-.75-1.3m.62-.02c-.14-.15-.22-.34-.31-.52.08.32.26.6.43.88l-.12-.36m10.945-2.382l-.07.15c-.1.76-.34 1.511-.69 2.212.4-.73.65-1.541.75-2.362M12.45.12c.27-.1.66-.05.95-.12-.37.03-.74.05-1.1.1l.15.02M3.006 5.142c.07.57-.43.8.11.42.3-.66-.11-.18-.1-.42m-.64 2.661c.12-.39.15-.62.2-.84-.35.44-.17.53-.2.83"/></svg> <svg role="img" fill="#262577" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>CentOS</title><path d="M12.076.066L8.883 3.28H3.348v5.434L0 12.01l3.349 3.298v5.39h5.374l3.285 3.236 3.285-3.236h5.43v-5.374L24 12.026l-3.232-3.252V3.321H15.31zm0 .749l2.49 2.506h-1.69v6.441l-.8.805-.81-.815V3.28H9.627zm-8.2 2.991h4.483L6.485 5.692l4.253 4.279v.654H9.94L5.674 6.423l-1.798 1.77zm5.227 0h1.635v5.415l-3.509-3.53zm4.302.043h1.687l1.83 1.842-3.517 3.539zm2.431 0h4.404v4.394l-1.83-1.842-4.241 4.267h-.764v-.69l4.261-4.287zm2.574 3.3l1.83 1.843v1.676h-5.327zm-12.735.013l3.515 3.462H3.876v-1.69zM3.348 9.454v1.697h6.377l.871.858-.782.77H3.35v1.786L.753 12.01zm17.42.068l2.488 2.503-2.533 2.55v-1.796h-6.41l-.75-.754.825-.83h6.38zm-9.502.978l.81.815.186-.188.614-.618v.686h.768l-.825.83.75.754h-.719v.808l-.842-.83-.741.73v-.707h-.7l.781-.77-.188-.186-.682-.672h.788zm-7.39 2.807h5.402l-3.603 3.55-1.798-1.772zm6.154 0h.708v.7l-4.404 4.338 1.852 1.824h-4.31v-4.342l1.798 1.77zm3.348 0h.715l4.317 4.343.186-.187 1.599-1.61v4.316h-4.366l1.853-1.825-.188-.185-4.116-4.054zm1.46 0h5.357v1.798l-1.785 1.796zm-2.83.191l.842.829v6.37h1.691l-2.532 2.495-2.533-2.495h1.79V14.23zm-1.27 1.251v5.42H8.939l-1.852-1.823zm2.64.097l3.552 3.499-1.853 1.825h-1.7z"/></svg> <svg role="img" fill="#294172" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Fedora</title><path d="M12.005.001C7.29-.064 2.744 2.962.957 7.317c-.885 1.953-1 4.116-.946 6.225.01 2.666-.02 5.335.015 7.999.113 1.542 1.654 2.614 3.134 2.44 3.257-.02 6.514.044 9.77-.034 4.79-.303 9.155-3.796 10.527-8.39 1.4-4.345.03-9.413-3.39-12.443A11.968 11.967 0 0012.005.001zm3.52 2.842c.406-.01.807.032 1.197.117.557.286.945.826.902 1.383-.058.75-.587 1.247-1.38 1.246a2.95 2.95 0 00-.72-.09c-1.725-.053-3.167 1.61-2.97 3.305.024.796-.044 1.601.033 2.392.333.457.987.18 1.475.256.276 0 .562.007.85.008a.134.134 0 00.042.008 1.29 1.29 0 011.29 1.295 1.29 1.29 0 01-1.298 1.295.14.14 0 00-.06.013c-.777.003-1.553 0-2.33.002-.066 1.616.197 3.276-.31 4.84-.865 2.706-3.97 4.337-6.685 3.62-.537-.284-.91-.79-.868-1.334.062-.796.656-1.308 1.532-1.24.075.006.113.012.168.02.716.14 1.477.034 2.107-.391 1.127-.645 1.502-1.977 1.396-3.193-.02-.687.043-1.384-.033-2.066-.333-.456-.984-.18-1.472-.256h-.8a.137.137 0 00-.066-.015 1.29 1.29 0 01-1.298-1.295c0-.72.574-1.29 1.29-1.295a.136.136 0 00.063-.016h2.316c.06-1.507-.159-3.046.213-4.523.648-2.376 2.952-4.12 5.415-4.086zm.705.052l.13.024zm1.224.28c1.444.543 2.636 1.706 3.25 3.12a81.141 81.136 0 01-2.903-2.592 1.762 1.762 0 00-.346-.527zm.45.935a86.96 86.954 0 002.966 2.596c-.014-.044-.033-.087-.049-.13.166.448.265.918.301 1.402a77.847 77.842 0 01-3.39-2.975l.015-.033c.088-.182.143-.386.16-.606a1.393 1.393 0 000-.197c0-.019-.002-.038-.004-.057zm.02.196c-.058.516-.058.516 0 0zm-.314.894a85.347 85.341 0 003.53 3.06c0 .071.015.14.013.21a4.94 4.94 0 01-.06.814A78.528 78.523 0 0117.011 5.7a1.56 1.56 0 00.6-.5zm.526 1.83c.898.8 1.856 1.623 2.918 2.485a5.215 5.215 0 01-.242.863 74.032 74.027 0 01-2.307-1.963l.002.055c0 .076-.004.152-.01.226.703.612 1.44 1.237 2.24 1.885-.109.26-.234.512-.38.752a73.605 73.6 0 01-2.06-1.752 3.13 3.13 0 00-.16-2.55zm.068 2.758c.644.56 1.32 1.131 2.043 1.72a5.868 5.867 0 01-.5.651 74.24 74.235 0 01-1.99-1.705c.185-.2.332-.425.447-.666zm-.603.816a82.545 82.54 0 002.005 1.71c-.19.2-.392.385-.607.556a75.858 75.853 0 01-2.043-1.77c.203-.108.396-.247.572-.425.027-.022.047-.047.073-.07zm-.86.594a84.37 84.365 0 002.088 1.8c-.23.17-.48.311-.734.444a79.06 79.054 0 01-2.256-2.005c-.024.002-.05.001-.074.003l-.026-.015a3.506 3.506 0 001.002-.227zm-10.046.248l-.02.014h-.013l.004.004a1.573 1.573 0 00-.51.55l-.45-.427a5.38 5.38 0 01.99-.14zm-.994.14l-.127.032.127-.03zm-.27.075l.622.592a1.55 1.55 0 00.008 1.045c-.468-.444-.94-.893-1.432-1.348.26-.118.529-.21.803-.289zm-1.03.4c.72.674 1.425 1.347 2.134 2.03a2.68 2.68 0 00-.826.242c-.64-.61-1.3-1.23-1.998-1.865a5.98 5.98 0 01.69-.406zm12.022.204c.473.42.958.846 1.474 1.285a5.37 5.37 0 01-.908.342l-.617-.498a1.55 1.55 0 00.05-1.13zm-12.901.345a140.01 140.01 0 011.958 1.844 3.158 3.158 0 00-1.527 2.332A98.329 98.323 0 001.777 14.8a5.574 5.573 0 00-.091.22c.777.698 1.522 1.391 2.252 2.087.002.284.042.57.14.848.053.19.13.366.22.533a107.243 107.236 0 00-2.86-2.635c.027-.108.04-.217.074-.323.137-.518.366-.998.65-1.44.69.621 1.35 1.241 2.002 1.86a2.98 2.98 0 01.107-.232 99.457 99.45 0 00-1.986-1.819c.15-.214.31-.42.49-.613A109.76 109.76 0 014.7 15.088c.054-.06.11-.122.168-.178a98.104 98.104 0 00-1.94-1.79 5.82 5.82 0 01.592-.509zm12.719.153l-.027.265zm1.733.754l-.05.023.05-.023zm-1.72.091l.44.354c-.287.06-.582.096-.88.103h-.016a1.56 1.56 0 00.457-.457zM1.386 16.101c1.32 1.189 2.532 2.357 3.78 3.559l.175.166c-.237.107-.44.266-.594.465A125.942 125.935 0 001.3 17.076a5.176 5.176 0 01.086-.976zm-.07 1.29A154.286 154.276 0 014.61 20.5c-.11.2-.18.428-.2.677a1.406 1.406 0 000 .166c-.92-.884-1.856-1.78-2.88-2.71a5.65 5.65 0 01-.215-1.243zm.378 1.714c.943.876 1.852 1.748 2.778 2.64l.02.02c.07.195.186.372.327.532-1.445-.56-2.562-1.767-3.125-3.192zm3.352.264c.179.14.374.26.58.358l-.025.005-.018-.015a3.014 3.014 0 01-.537-.348zm1.047 3.252l.15.025zm.228.04s.09.007.274.025a33.671 33.669 0 00-.274-.026z"/></svg> <svg role="img" fill="#EE0000" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Red Hat</title><path d="M16.009 13.386c1.577 0 3.86-.326 3.86-2.202a1.765 1.765 0 0 0-.04-.431l-.94-4.08c-.216-.898-.406-1.305-1.982-2.093-1.223-.625-3.888-1.658-4.676-1.658-.733 0-.947.946-1.822.946-.842 0-1.467-.706-2.255-.706-.757 0-1.25.515-1.63 1.576 0 0-1.06 2.99-1.197 3.424a.81.81 0 0 0-.028.245c0 1.162 4.577 4.974 10.71 4.974m4.101-1.435c.218 1.032.218 1.14.218 1.277 0 1.765-1.984 2.745-4.593 2.745-5.895.004-11.06-3.451-11.06-5.734a2.326 2.326 0 0 1 .19-.925C2.746 9.415 0 9.794 0 12.217c0 3.969 9.405 8.861 16.851 8.861 5.71 0 7.149-2.582 7.149-4.62 0-1.605-1.387-3.425-3.887-4.512"/></svg> <svg role="img" fill="#1793D1" width="12" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><title>Arch Linux</title><path d="M11.39.605C10.376 3.092 9.764 4.72 8.635 7.132c.693.734 1.543 1.589 2.923 2.554-1.484-.61-2.496-1.224-3.252-1.86C6.86 10.842 4.596 15.138 0 23.395c3.612-2.085 6.412-3.37 9.021-3.862a6.61 6.61 0 01-.171-1.547l.003-.115c.058-2.315 1.261-4.095 2.687-3.973 1.426.12 2.534 2.096 2.478 4.409a6.52 6.52 0 01-.146 1.243c2.58.505 5.352 1.787 8.914 3.844-.702-1.293-1.33-2.459-1.929-3.57-.943-.73-1.926-1.682-3.933-2.713 1.38.359 2.367.772 3.137 1.234-6.09-11.334-6.582-12.84-8.67-17.74zM22.898 21.36v-.623h-.234v-.084h.562v.084h-.234v.623h.331v-.707h.142l.167.5.034.107a2.26 2.26 0 01.038-.114l.17-.493H24v.707h-.091v-.593l-.206.593h-.084l-.205-.602v.602h-.091"/></svg>
You can simply get the libraries using the **sudo apt install** command in Debian Linux, Ubuntu.

Just do:
```
sudo apt install g++
sudo apt install libcurl4-openssl-dev
sudo apt-get install libncurses5-dev libncursesw5-dev
```
Or in M**acOS**, first install [Homebrew](https://brew.sh/) and then do:
```
brew install g++
brew install make
brew install curl
brew install ncurses
```
Or if you are using **Red Hat linux**, **CentOS** or **Fedora Linux 21 or older** do:
```
sudo yum group install "Development Tools"
sudo yum install libcurl
sudo yum install ncurses-devel
```
Or if you are using **Fedora Linux 22.x+** do:
```
sudo dnf install gcc-c++
sudo yum install libcurl
sudo dnf install ncurses-devel
```
Or if you are using **Arch linux** or **Manjaro** do:
```
sudo pacman -Sy gcc
sudo pacman -S curl
sudo pacman -S ncurses
```
The header files are platform-independent and hence the above commands will install/update your libraries.
## **Todo license**
Check the [LICENSE](https://github.com/GhostVaibhav/Todos/blob/master/LICENSE) file
## **The curl license**
Copyright &copy; 1996 - 2021, Daniel Stenberg, [daniel@haxx.se](mailto:daniel@haxx.se), and many contributors, see the THANKS file.

All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization of the copyright holder.

## **JSON for Modern C++ license**
Copyright &copy; 2013-2021 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## **PDCurses**
Special thanks to the maintainer of the repository William McBrine [wmcbrine@gmail.com](mailto:wmcbrine@gmail.com) and Chris Szurgot [szurgot@itribe.net](mailto:szurgot@itribe.net) for porting it to the Windows console.

## **Ncurses license**
Copyright &copy; 2018-2019,2020 Thomas E. Dickey

Copyright &copy; 1998-2016,2017 Free Software Foundation, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, distribute with modifications sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:                
                                         
The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software. 
                                         
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright holders shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization.