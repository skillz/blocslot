# Blocslot

This repo demonstrates a simple Skillz SDK integration against the Marmalade
 Blocslot example project.

![screenshot](Screenshot.png)

# Building

This project is setup to work with the latest version of the Skillz iOS SDK,
which can be found at [https://skillz.com/developer/downloads](https://skillz.com/developer/downloads).

* Copy the `SkillzSDK` directory from the `Marmalade` directory in the Skillz SDK.

The Skillz SDK does not work in the Marmalade simulator so the project will also
need to be configured to run directly on a device. The project needs the following:

* A custom bundle ID
* A provisioning profile for the bundle ID

Further details can be found in [Marmalade's documentation](http://docs.madewithmarmalade.com/display/MD/Deploying+to+and+testing+on+iOS+devices).

For additional Skillz integration details please refer to the 
[Skillz documentation](https://developers.skillz.com/developer/docs/install_framework_ios_marmalade).

# License

The Blocslot code and assets are property of Marmalade and are provided here for
educational purposes with explicit permission from Marmalade.

