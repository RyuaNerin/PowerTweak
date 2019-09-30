# PowerTweak

- Simple tweak for power key in keyboard

- Expected to be available on laptops that have a power key on the keyboard.

- Tested in `Windows 10` and those devices

	- `Acer Aspire3 A315-42 (에이서 아스파이어 A315-42 체인지)`

- [GPLv3](LICENSE)

## Download

1. Download `PowerTweak.zip` in [this page]() to appropriate location.

2. Unzip downloaded from **1**

	|File Name|Description|
	|:-:|-|
	|`PowerTweak.exe`|Main executable|
	|`install.bat`|Enable autorun at Windows startup|
	|`uninstall.bat`|Disable autorun|

## Usage

- Run `PowerTweak.exe`

- Run `Install.bat` to start automatically with windows.
	
	- Do not remove or move `PowerTweak.exe`

    - if want no more, run `uninstall.bat`

- Press power key in keyboard.

	|Pressed count in 1 second|Action|
	|:-:|:-:|
	|1|Off screen|
	|2|Lock user|
	|3|Sleep System|

- Commandline arguments

	|Argument|Description|
	|-|-|
	|`/install`|Enable autorun at Windows startup|
	|`/uninstall`|Disable autorun|