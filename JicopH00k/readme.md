# BOF Template

This repository is meant to host the core files needed to create a Beacon Object File for use with Cobalt Strike.
A Beacon Object File (BOF) is a compiled C program, written to a convention that allows it to execute within a Beacon process and use internal Beacon APIs. BOFs are a way to rapidly extend the Beacon agent with new post-exploitation features.


#### hook
Directory containing the example hook BOF which hooknstrates items that are now supported in Cobalt Strike version 4.7
- JicopH00k.c   - source code for the hook example.
- hook.cna - aggressor script file to execute the hook command.

Use:
- Use one of the build scripts to build the object file.
- Load the script - In Cobalt Strike -> Script Manager -> Load `hook/hook.cna`
- Execute the `hook` command in the beacon console.


compile
````
  i686-w64-mingw32-gcc JicopH00k.c -o JicopH00k.o -lws2_32
````

## tests

The tests directory contains examples for using the internal Beacon APIs.  The directory contains the following:

- hook.cna            - Aggressor script file for running the tests
- src directory       - Contains example source files for using the internal Beacon APIs.


-------------------------------------------------------------------------------------------------------------------------


## Usage

Aggressor script included with a command line function and a menu item for more complicated requests.

### Examples

Simple request to confirm a 200 response:

```
beacon> evillog https://f-secure.com
[*] Running Simple Web Request Utility (@ajpc500)
[+] host called home, sent: 2882 bytes
[+] received output:
GET f-secure.com:443 
User Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36
Accept: */*

[+] received output:
Response Code: 200
```

Simple GET request that prints output (print is useful for calls to web APIs, a bad idea against a Javascript-heavy webpage!):

```
beacon> evillog http://example.com 80 GET --show
[*] Running Simple Web Request Utility (@ajpc500)
[+] host called home, sent: 2880 bytes
[+] received output:
GET example.com:80 
User Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36
Accept: */*

[+] received output:
Response Code: 200

[+] received output:
<!doctype html>
<html>
<head>
    <title>Example Domain</title>
