# uap-tcl

This is the Tcl implementation of the [User Agent String Parser](https://github.com/ua-parser) project.

## Requirements

Supported systems:

- Linux
- MacOS

## Installation

```bash
git clone https://github.com/jerily/uap-tcl.git --recurse-submodules
cd uap-tcl
mkdir build
cd build
cmake ..
# or if TCL is not in the default path (/usr/local/lib):
# change "TCL_LIBRARY_DIR" and "TCL_INCLUDE_DIR" to the correct paths
# cmake .. -DTCL_LIBRARY_DIR=/usr/local/lib -DTCL_INCLUDE_DIR=/usr/local/include
make
make install
```

## Usage

### Package initialization

User agent strings are parsed using multiple regular expressions. By default, regular expressions are not compiled when the package is loaded and will be compiled at runtime as needed. This can cause an unexpected and significant delay in processing the first strings.

To avoid this delay, it is possible to use the **::uap::init** command after the package has been loaded. This command will pre-compile all regular expressions and eliminate the delay in the first parses.

### Parsing

To process user agent strings, a single command **::uap::parse** is used, which has the following format:

- **::uap::parse string ?type?**

It returns a Tcl dictionary with values.

If **?type?** is not specified, then returned dictionary will contain 4 keys:

- **ua** - information of the family type of the User-Agent and its version if available
- **os** - information of the os type of the Operating System (OS) the User-Agent runs and its version if available
- **device** - information of the device family the User-Agent runs on
- **string** - the original string passed to the input

The value by key **ua** is a nested dictionary with the following keys:

- **family** - specifies the User-Agents family
- **major** - major version number/info of the family
- **minor** - minor version number/info of the family
- **patch** - patch version number/info of the family
- **patch_minor** - patch minor version number/info of the family

Any of the keys **major**, **minor**, **patch** and **patch_minor** may be missing if this information is not available.

If the User-Agent family is unknown, the value of the **family** key will be `Other`.

The value by key **os** is a nested dictionary with the following keys:

- **family** - specifies the OS
- **major** - major version number/info of the OS
- **minor** - minor version number/info of the OS
- **patch** - patch version number/info of the OS
- **patch_minor** - patch minor version number/info of the OS

Any of the keys **major**, **minor**, **patch** and **patch_minor** may be missing if this information is not available.

If the OS is unknown, the value of the **family** key will be `Other`.

The value by key **device** is a nested dictionary with the following keys:

- **family** - specifies the device family
- **brand** - the manufacturer of the device
- **model** - the model of the device

Any of the keys **brand** and **model** may be missing if this information is not available.

If the device is unknown, the value of the **family** key will be `Other`.

For example:

```
package require uap

set ua_string {Mozilla/5.0 (Linux; Android 11; SM-A225M) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/85.0.4183.127 Mobile Safari/537.36 Instabridge/22}
puts [::uap::parse $ua_string]
```

returns:

```
ua {family Instabridge major 22} os {family Android major 11} device {family {Samsung SM-A225M} brand Samsung model SM-A225M} string {Mozilla/5.0 (Linux; Android 11; SM-A225M) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/85.0.4183.127 Mobile Safari/537.36 Instabridge/22}
```

If only specific information is required, it can be obtained with the second argument **type**, which can be either `ua`, `os` or `device`. In this case, only a Tcl dictionary with the specified type of information will be returned.

For example:

```
package require uap

set ua_string {Mozilla/5.0 (Linux; Android 11; SM-A225M) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/85.0.4183.127 Mobile Safari/537.36 Instabridge/22}
puts [::uap::parse $ua_string ua]
```

returns:

```
family Instabridge major 22
```

Another example with information for the operating system only:

```
package require uap

set ua_string {Mozilla/5.0 (Linux; Android 11; SM-A225M) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/85.0.4183.127 Mobile Safari/537.36 Instabridge/22}
puts [::uap::parse $ua_string os]
```

returns:

```
family Android major 11
```

Another example with information for the device only:

```
package require uap

set ua_string {Mozilla/5.0 (Linux; Android 11; SM-A225M) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/85.0.4183.127 Mobile Safari/537.36 Instabridge/22}
puts [::uap::parse $ua_string device]
```

returns:

```
family {Samsung SM-A225M} brand Samsung model SM-A225M
```
