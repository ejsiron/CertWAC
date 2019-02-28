# Windows Admin Center Certificate Selector

Tool for easy selection of certificates to use in Windows Admin Center

## BETA NOTICE

This tool is currently in its first public beta. Please use caution if trying in production environments.

## Compatibility and Requirements

The Windows Admin Certificate Selector requires:

* Windows Server 2016 or later (including Core and Semi-Annual Channel), any edition
* An installation of [Windows Admin Center](https://www.microsoft.com/en-us/cloud-platform/windows-admin-center) in gateway mode
* A certificate in the local computer store with an Enhanced Key Usage of Server Authentication. It will work with self-signed certificates.
* Local administrative privileges
* *Recommended*: For the MSI package distribution only, an installation of the [Microsoft Visual C++ Redistributable for Visual Studio 2017](https://visualstudio.microsoft.com/downloads/) (look under the **Other Tools and Frameworks Section**). See the section on the MSI package for more information.

## Packages

Choose one of the two optional release packages

### Standalone EXE

This distribution has all necessary supporting DLLs linked in so that it will run without supporting files. This is the recommended package for one-off usage.

### MSI package

This distribution packages supporting DLLs separately. If your system already has the full Visual Studio C++ 2017 x86 redistributable package, Windows Update will keep them current. This is the recommended package for long-term installations.

The installer also makes these changes (configurable):

* Modification of the PATH environment so that you can run **CertWAC.exe** from any prompt.
* A shortcut icon on the Start menu

## Usage

WAC Certificate Selector presents a drop-down list of the available computer certificates. It will show details of the selected certificate. In order to be eligible, a certificate must meet all of the following criteria:

* Within validity period
* Have the *Server Authentication* Enhanced Key Usage
* The system must have the matching private key

If all of the above are true, and a gateway installation of Windows Admin Center is detected, the **OK** button will be active.

**Warning**: The reconfiguration stops the Windows Admin Center service. Windows Admin Center will be down and unavailable while MSI installs the certificate.

## How It Works

WAC Certificate Selector calls on Windows Admin Center's own MSI install package using the [/qb](https://docs.microsoft.com/en-us/windows/desktop/Msi/command-line-options) option to install the certificate. Upon clicking OK, you will see the Windows Installer progress bar.

Additionally, it appears that the Windows Admin Center MSI package is malformed with the /qb option, as it leaves the service in the stopped state. At the end of the installation, WAC Certificate Selector will instruct the system to start the service.

## Building from source

To build the source code included in these projects without modification, you will need a copy of [Visual Studio 2017](https://visualstudio.microsoft.com/downloads/) and a copy of the [WiX Toolset 3.11.1](http://wixtoolset.org/). Visual Studio must be configured for C++ development and include the Windows 10 SDK.