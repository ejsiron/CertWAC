# Windows Admin Center Certificate Selector

Graphical tool for easy selection of certificates to use in Windows Admin Center

## Archive Notice

Starting with Windows Admin Center version 2410 (v2), Microsoft's installer includes a drop-down box for selecting the certificate (it works much like this app). That means that this app does not currently provide value. If you want to look at some code for accessing PKI certificates on Windows using C++ and Win32, you might find some use for the code.

Thanks everyone for supporting this project!

## Brief

[Windows Admin Center](https://www.microsoft.com/en-us/cloud-platform/windows-admin-center) (WAC) is a powerful tool that allows you to monitor and maintain your Windows systems via a convenient HTML 5 interface. It uses a PKI certificate to encrypt your connection to its web interface. Unfortunately, it lacks an intuitive, simple interface for selecting which certificate to present. You must drill through the installed certificates to find the one that you want, copy the _Thumbprint_ value to a plain-text tool to clear out invalid characters, start the WAC installer, and paste in the thumbprint. The procedure is especially unpleasant if you installed WAC on Windows Server Core. [Microsoft has closed the UserVoice request to make this simpler](https://windowsserver.uservoice.com/forums/295071-management-tools/suggestions/33950335-how-do-windows-admin-center-change-certificate), indicating that they currently have no intent to make this easier.

The WAC Certificate Selector neatly solves the usability problem. If you have WAC installed and a valid certificate in the local computer certificate store, you select it and let WAC handle the rest.

## BETA NOTICE

This tool is currently in public beta. Please use caution if trying in production environments.

## Compatibility and Requirements

The Windows Admin Certificate Selector requires:

- Windows Server 2016 or later (including Core and Semi-Annual Channel), any edition
- An installation of [Windows Admin Center](https://www.microsoft.com/en-us/cloud-platform/windows-admin-center) in gateway mode. Only GA versions are supported, although it might work with preview builds.
- A certificate in the local computer store with an Enhanced Key Usage of Server Authentication. It will work with self-signed certificates.
- Local administrative privileges
- _Recommended_: For the MSI package distribution only, an installation of the [Microsoft Visual C++ Redistributable for Visual Studio 2017](https://visualstudio.microsoft.com/downloads/) (look under the **Other Tools and Frameworks Section**). See the section on the MSI package for more information.

## Packages

You can download release builds from the [Releases page](https://github.com/ejsiron/CertWAC/releases). Choose one of the two packages:

### Standalone EXE

This distribution has all necessary supporting libraries statically linked so that it will run without supporting files. This is the recommended package for short-term one-off usage. Note that because of the static linking, Windows Update will not patch any flaws in the supporting libraries.

### MSI package

This distribution packages supporting DLLs. If your system  has the full Visual Studio C++ 2017 x86 redistributable package, Windows Update will keep them current. This is the recommended package for long-term installations.

The installer also makes these changes (configurable):

- Modification of the PATH environment so that you can run **CertWAC.exe** from any prompt.
- A shortcut icon on the Start menu

## Usage

If you have not yet installed Windows Admin Center, run its installer. When prompted for certificate information, allow WAC to generate a self-signed certificate. Next, request a certificate from your provider, whether a PKI operated internally by your organization or a public PKI certificate retailer. The certificate must have the _Server Authentication_ Enhanced Key Usage. Most providers include that in _Web Server_ templates. Install the certificate into the local computer certificate store.

Once you have WAC installed and a certificate prepared, run WAC Certificate Selector. If you obtained the MSI installation package, it will optionally place a shortcut on your Start menu and add its program folder to the PATH. If you have the standalone EXE, run **CertWAC.exe** from the location where you placed it.

WAC Certificate Selector presents a drop-down list of the available computer certificates. It will show details of the selected certificate in the large text box. In order to be eligible, a certificate must meet all of the following criteria:

- Within validity period
- Have the _Server Authentication_ Enhanced Key Usage
- The system must have the matching private key

If all of the above are true, and a gateway installation of Windows Admin Center is detected, the **OK** button will be active.

_Optional_: WAC Certificate Selector includes an option to change the port that WAC listens on.

**Warning**: The reconfiguration stops the Windows Admin Center service. Windows Admin Center will be down and unavailable while MSI installs the certificate.

## How It Works

WAC Certificate Selector calls on Windows Admin Center's own MSI install package using the [/qb](https://docs.microsoft.com/en-us/windows/desktop/Msi/command-line-options) option to install the certificate. Upon clicking OK, you will see the Windows Installer progress bar. WAC invokes MSI following [Microsoft's official documentation](https://docs.microsoft.com/en-us/windows-server/manage/windows-admin-center/deploy/install) with one exception: WAC's installer ignores any entered value for SME_PORT (you can verify by reviewing the logs). Therefore, WAC Certificate Selector will modify the related registry key prior to invoking MSI. The installer will read the updated value and apply it while changing the certificate.

Additionally, it appears that the Windows Admin Center MSI package is malformed with the /qb option, as it leaves the service in the stopped state. At the end of the installation, WAC Certificate Selector will instruct the system to start the service.

## Building from source

To build the source code included in these projects without modification, you will need a copy of [Visual Studio 2017](https://visualstudio.microsoft.com/downloads/) and a copy of the [WiX Toolset 3.11.1](http://wixtoolset.org/). Visual Studio must be configured for C++ development and include the Windows 10 SDK.
