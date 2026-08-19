# EQMosaiq

## Disclaimer

This tool got my accounts banned recently, this code dump is mainly to serve as an example for people who would want to make something similar.

This tool uses DLL injection to:
* Hook into the DX11 Present function to catpure the running instance screen into a shared dx11 texture. This could be replaced by windows own capture system to avoid controversy
* Uses a similar offset system and auto login system that MQ uses
* Overrides files APIs to redirect eqini files so each instance can have its own settings



