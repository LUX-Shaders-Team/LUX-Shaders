<img src="https://github.com/user-attachments/assets/baa74c07-c669-4d25-82f0-01201567808e"
     alt="800"
     width="400">

# LUX for Source SDK 2013

**LUX** is a Shader Modification for the [Source Engine](https://developer.valvesoftware.com/wiki/Source).<br>
It serves as a drag & drop `materialsystem\` Replacement on the **[Source Engine SDK (2013)](https://developer.valvesoftware.com/wiki/Source_2013)**.<br>
<br>

## Features
For more specific Details, see **[LUX Github Wiki]()**.
<details>
<summary> Total Overhaul of the materialsystem\ Folder </summary>
Implementing LUX is intended to be as easy as possible.<br>
To ensure this, all Changes have been restricted to the `materialsystem\` Folder.<br>
<br>
Using Code from the Orange Box and Alien Swarm,<br>
`materialsystem\shaderlib` has been reimplemented.<br>
On LUX, it has been heavily modified to allow for some of the new Features.<br>
Previously LUX was limited to the `materialsystem\stdshaders` Folder.<br>
Due to the ShaderLib Implementation, it is now restricted to `materialsystem\`<br>

</details>

<details>
<summary> Faster Shader Compiles </summary>

Large Portions of and entire Shaders have been rewritten.<br>
To keep Visuals the same as Stock, often times they are forced to use the same mathematical Functions.<br>
This didn't mean their Systems couldn't be entirely reevaluated.<br>
<br>
All of the Shaders in **LUX** combined can be compiled in ~15 Minutes, even on dated Hardware.<br>
These are some of the CPU's LUX has been compiled with, in well under 15 Minutes:
* R7-7530u
* AMD Fx8350
* i7-6700k
* i7-5600U

Newer Hardware (R9-5950x), can go down to as low as 3-4 Minutes.<br>
<br>
Various Shaders now also have their own .bat Scripts.<br>
If a specific Shader is being modified, it will be faster to compile the subset of Shaders instead.<br>
This is an Issue alluded to in the Stock common_ps_fxc2.h
<code>
// This file is here so you can add new utility functions without
// changing common_fxc.h and causing a recompile of the entire universe.
</code>
<br>
In Addition to this, Header Files are now more or less minimalistic.<br>
This avoids the constant parsing of monolithic File Structures, often filled with unused Functions, on every Shader Combo.<br>
There are some Exceptions to this like the LUX EnvMap Header which has a lot of Functions to handle the large Amounts of Features that EnvMaps support..<br>
If you write new Shaders, splitting Features by Headers and Functions is suggested.<br>

</details>

<details>
<summary> Compatibility with Mapbase </summary>

**LUX** aims to be a viable Replacement for Mapbase Shaders.<br>
Because of this, specific Features have been added to accomodate it.<br>
Some of the Features from Mapbase require modifying Client.dll and/or Compilers.<br>  
If you are planning on using a Feature otherwise only available with Mapbase you will have to port these yourself.
<br>
* Parallax Corrected Cubemaps require a custom VBSP.
* Treesway requires a custom Client.dll on SDK2013SP.
* Radial-Fog only controllable via ConVar on SDK2013SP.
<br>
To facilitate LUX working on Mapbase,<br>
every Shader has SDK_ Fallbacks that lead to the LUX_ Variants.<br>

</details>

<details>
<summary> Selective Stock Shader Replacement </summary>

Anyone who has touched the TF2SDK in recent Days,<br>
will have noticed that the VPC Setup isn't exactly great.<br>
Some People had Issues compiling Shaders like Core for ps20b because of new Instructions..<br>
To replace a Shader, it has to be added to the Project in VPC.<br>
This has two major Downsides.<br>
1. Need to regenerate the Solution File, everytime a new Shader is to be replaced.
2. Some Shaders use other Shaders as a dependency.<br>
This _sometimes_ makes it impossible to be selective.<br>
<br>
**LUX** solves both of these Issues.<br>
It provides a Headerfile with Defines that allow for the selective replacing of Shaders (Among other Things).<br>
By Default all available Shaders are contained in the Project.<br>
Having to mess with the VPC Scripts to add an existing Shader is not necessary.<br>
<br>
**NOTE**:<br>
SDK2013SP does not allow for the replacement of Stock Shaders.<br>
Using Various Methods ( Memory Patches, the Material Passthrough Method ),<br>
the replacement Functionality may be used on SDK2013SP.<br>
Replacing Stock Shaders is optional, all LUX Shaders are using the LUX_ Prefix, Stock Shader Replacement is handled via Fallback Shaders.<br>
The same goes for SDK_ Shaders. They fallback to LUX_.<br>

</details>

<details>
<summary> Shadermodel 3.0 only </summary>

To allow for many of the new Features, all of the Shaders now use **Shadermodel 3.0**.  
A Limitation created by this is that Linux on  **Source 2013 Singleplayer** cannot run LUX.<br>
This is a ToGL Limitation ( Translation-Layer ).<br>
Support for SM3.0 on ToGL was added in Team Fortress 2,<br>
but it never made it's way to SDK2013SP.<br>
* **DXVK** may serve as an alternative here for Linux Users.<br>
* **SM3.0** released in 2004, Valve considered **ps2.0b** low end hardware in **~2008**.<br>
* Three DirectX Versions have released since DX9.<br>
* The Shadercompiler on the TF2SDK doesn't even allow Shaders below < ps20b anymore.<br>
Meaning you couldn't add support for DX8 if you wanted to.<br>
<br>
Older hardware that is not capable of running SM3.0 will not be able to use the new Shaders.<br>
There are no Fallbacks for DX8 and DX7 and DX6.<br>
NOTE: If you have an Intel integrated CPU that DOES support SM3.0 but has Issues, run with  -force_vendor_id 0x10DE -force_device_id 0x1180
</details>

<details>
<summary> Documentation on Shaders and Parameters
</summary>

Due to the Development of **LUX**, Articles on the **VDC** such as `$lightwarptexture`, `$detail`, `UnlitTwoTexture` and `$envmapmask`
received massive updates, detailing new information about Caveats and Bugs.<br>
**LUX** Shaders themselves are heavily commented - sometimes more than necessary..<br>
Structural Comments ( Section Separation ) have been added to every Shader to clearly differentiate the different Systems.<br>
Draw Functions have been organised to reflect this Change.<br>
Shaders now follow clearly defined Patterns/Layouts, which makes it easier to find Things in a Shader and to make new ones by following the provided Structure.<br>
Instances where some whacky Behaviour from Stock-Shaders has to be considered to keep visual consistency, have been commented with "Stock-Consistency".<br>
<br>
On this Repositories Wiki you can find a List of all Shaders and their Parameters.<br>
It also details how you can write new Shaders in the LUX Environment.<br>
Porting existing Shaders is not recommended.<br>
The whole Point of LUX is to remove all the old Things and get a fresh Start.<br>
If you choose to adapt a Shader to LUX anyways, please reach out if you need Help or share the Things you had to change and how so we can (try to) document this on the Wiki.<br>
<br>
Several Console Commands have been added to help with Documentation.<br>
One of them allows dumping Information about Shaders in the .dll in a markdown ready Format. ( See also the Wiki )<br>
Shaders now list in ( to varying degrees of Detail ) what Geometry they are made to support and what Limitations apply.<br>
<br>
Helper Strings have been added to *all* Parameters.<br>
They are massively more documented compared to their stock Variants.<br>
Texture Parameters detail their Channel-Usages, other Parameters define their Size and Type.<br>
Internal Shader Documentation can be looked at using the in-game Console as well. ( See also the Wiki )<br>

</details>

<details>
<summary> Features previously unavailable to Source 2013 </summary>

Various Features from other Branches have been implemented or recreated.<br>
This includes older Features, such as `$SelfIllumTexture` and `$EnvMapSphere`.<br>
Newer Features like `$PhongAlbedoBoost` (only in CS:GO) have been approximated.<br>
**NOTE**:
Features like `$SSBumpMathFix have been derived from visual research and logical conclusions, using the Games the Features are from as a visual Reference.<br>
( Left 4 Dead 2, CS:GO, Portal 2 .. )<br>
A lot of People helped in this Endeveaour.<br>
by providing Reference Images, testing these Features and comparing to Branches with these Features.<br>
Some even ported entire Maps from Left 4 Dead for their Mod Projects,<br>
Going as far as replicating the common Infected themselves from both L4D1 and L4D2.<br>

</details>

<details>
<summary> Fixing Bugs, Caveats, and missing Combos </summary>

**LUX** addresses *many* of the Issues Stock Shaders are known for.<br>
From straight up missing or broken Features, to whacky stock Behaviour.<br>
There is a lot to cover, so much that it's not possible to representative Examples.<br>
<br>
If there is something you want fixed, check if it is!<br>
**NOTE:** Some fixes may be locked behind certain Material Parameters ( like $PhongNewBehaviour ).<br>
Going through the Parameter List for the Shader is recommended.<br>

</details>

## VERY IMPORTANT:
**(26.01.2026 DMY):**<br>

SM30 Constant Registers don't work as intended in either SDK's<br>
We have gotten **multiple** Confirmations that this will get fixed.<br>
Spoiler: Among other Issues ( **Cough** Save-Restore ) this hasn't been fixed after **Months**.<br>
Release will not be delayed by empty Promises, here's what this means for LUX Users.<br>

Until this gets fixed for the TF2SDK:<br>
* Your Game crashes on Launch if UnlitGeneric uses Registers above c31.<br>
Lux UnlitGeneric does not use Registers above c31 to account for this.<br>
* Your Game crashes on Exit if any Shader used above c31.<br>
The old SDK2013 will crash if you use Registers above c31 at all.<br><br>

This can be resolved *right now* for both SDK's by overriding Hardware Caps using Memory patching<br>
LUX has a no-RE and no-Leak Rule thefore this is not provided with the LUX Repo. Sorry!<br>

You can find the Patch for SDK2013SP at **[SDK2013CE](https://github.com/Nbc66/source-sdk-2013-ce)**.<br> 
You can find the Patch for the TF2SDK at **[Srcbox](https://github.com/Zeldaboy14/Srcbox_2025/commit/4840b49ec89b5a5227ec49324faf5f94cdedb201)**<br>

## Build instructions
Steps for building **LUX**:  
1. Run ```createallprojects.bat ``` to generate the `.sln` Solution File.
2. Compile the `game_shader_generic_example.dll` using the 'Shaders' Project in the .sln File.<br>
( This results in a `game_shader_dx9.dll` for SDK2013SP )<br>

3. Run `src\materialsystem\stdshaders\!Compile_All.bat` to compile all the Shaders.<br>
The Time this takes depends on your Hardware. This Step is CPU intensive.<br>

4. Copy the compiled Shaders found in `game\lux\shaders\fxc\` to your `mod\shaders\fxc\` Folder.<br>
And the previously mentioned .dll File from `game\lux\bin\` your `mod\bin\` Folder.<br>

The copying Process can be automated.<br>
To do so, the Destination Folder in the $Compile .bat Files and the VPC Scripts need to be adjusted.<br>
You may also choose to symlink these Files instead.<br>
<br>

---

Steps to implement **LUX** into an existing Project.<br>
1. Replace the `materialsystem` Folder in your `src\` with the one from LUX.
2. Open `materialsystem/stdshaders/ShadersBuildDirectories.bat`<br>
There change `targetdir` and `GAME_DIR` so it matches your Mods Name and Path.<br>
If you have a custom .exe Filename, add it to the `PROCESS_LIST` here as well.<br>
`PROCESS_LIST` is only used for .vcs Reloading, so this is mostly optional.
3. Follow Steps from `building LUX`above

**NOTE1:**<br>
For the Shaders Project the .vpc Files are contained within `stdshaders`.<br>
LUX provides the .vpc Scripts for both SDK2013SP, old SDK2013MP **&** the TF2SDK.<br>
This means no Changes are necessary to the VPC Scripts to have it in your Solution.<br>
If at some Point you have decided to change the Names of your Shader VPC Scripts,<br>
you will have to modify the stdshader ones to account for that!<br>

**NOTE2:**<br>
Keeping some kind of commit-history between old and new Shaders is basically impossible.<br>
Due to how LUX was made & designed, all old Shader Files are just gone, nuked, kablamo.<br>
They may also be relocated, flipped upside down or renamed.<br>
It is recommended you nuke `materialsystem` in it's entirety, commit it and then push LUX as a secondary commit.<br>

## Technical
For developing with **LUX** either as a Programmer, Artist or both see also:<br>
 - **[LUX Shader Wiki]()**<br>
 - **[LUX replaced Shaders Table]()**<br>
 - **[Debugging Shaders]()**<br>
 - **[Writing new Shaders on LUX]()**<br>

## Additional Resources
* Vanilla `vbsp.exe` does not recognize custom Shaders.<br>
This causes `vrad.exe` to not compile Lightmaps for custom Shaders.<br> This is fixed in Forks like **[Mapbase](https://github.com/mapbase-source/source-sdk-2013)** and **[vbsp++](http://ficool2.github.io/HammerPlusPlus-Website/tools.html)**. 
* For Parallax corrected Cubemaps, `vbsp.exe` must be modified.<br>
See **[Parallax corrected Cubemaps](https://github.com/mapbase-source/source-sdk-2013/commit/cba218f7c2c17965eb53f2a2c0b847d4c43db4c2)** changes for VBSP from Mapbase.
 - **[Creating Shaders in the Source SDK 2013](https://developer.valvesoftware.com/wiki/Source_SDK_2013:_Shader_Authoring)**
 - **[LUX Valve Developer Wiki Page](https://developer.valvesoftware.com/wiki/LUX)**

## Contact
* Join the **[LUX Discord Server](https://discord.gg/cGv8GGSkpc)**

## Legal
Any Game or Modification that incorporates **LUX** or any Portion of its Codebase is required to include and redistribute the **`thirdpartylegalnotice.txt`** File.

---
## Contributing
Thank you for your interest in **LUX**!<br>
LUX is open to Contributions via the Pull-Request System.<br>
By submitting a Pull-Request, you agree to comply with the **[SOURCE 1 SDK LICENSE](LICENSE)**.<br>
Before making *any* Contribution, read the **[License Section](###license)**.<br>

Proposed Contributions that violate the Terms of the **[SOURCE 1 SDK LICENSE](LICENSE)** or the **[STEAM SUBSCRIBER AGREEMENT](https://store.steampowered.com/subscriber_agreement)** will *not* be accepted.<br>
This includes, but is not limited to:
- Using leaked Code.<br>
- Unauthorized use of Code.<br>
- Reverse-engineered Code.<br>

If your Contribution includes third-party Code, you **must** provide Attribution in accordance with that Code's License.<br>
If possible the use of third-party Code should be avoided altogether.<br>

---
## Credits  
Contributors refers to all Individuals or Entities that have submitted Code, Documentation, or other Materials to the **LUX** Project, during its Creation, either directly or indirectly.<br>
Each Contributors work is provided under the same License Terms as the **LUX** Project itself.<br>
Contributors are not legally liable or responsible for any Consequences, Damages, or Issues arising from the use, modification, or distribution of the **LUX** Codebase.<br>
Some **LUX** Contributors have chosen to remain anonymous; They are not listed below.<br>
If your contribution to the Project is not listed below, don't hesitate to get in contact.<br>

### Major Contributors
 - **[ShiroDkxtro2](https://github.com/WhiteRedDragons)**: Creator of **LUX**.
 - **[Trico Everfire](https://github.com/Trico-Everfire)**: Tools, Research.
 - **[Unusuario2](https://github.com/Unusuario2)** : GitHub README, Wiki, Shader Descriptions, and the ConCommand GitHub Wiki Dumper.   
 - **[Azzy]()**: C++ Contributions.
 - **[InevitablyDivinity](https://github.com/InevitablyDivinity/)**: C++ Contributions, Linux Testing and general Source Engine-related Support.
 - **[Kerim 'Nbc66' Čamdžić](https://github.com/Nbc66)**: C++ Contributions, Github Setup.
 - **[Zeldaboy14](https://github.com/Zeldaboy14)**: Tester on **SDK2013SP** and **SDK2013MP** and **TF2SDK**.
 - **[Fortress Connected](https://store.steampowered.com/app/3561600/Fortress_Connected/)**: Testing Maps from various Source Branches.
 - **[Team Fortress 2 Classified](https://store.steampowered.com/app/3545060/Team_Fortress_2_Classified/)**: Critical Role in Testing.

 ### Minor Contributors
 - **MrFunreal**: Provided their Infected Shader Documentation and Suggestions for **LUX** ($BlendModulateTransparency, $FlipNormal for the Refract Shader).
 - **MrKleiner**: Tools & Research.
 - **Krispy**: Tester on SDK2013MP before the TF2SDK.
 - **[Jay]**: Tester.
 - **URAKOLOUY5 ( Den Urakolouy )**: Tester.
 - **Dana Cief**: Materials used in Testing.
 - **.HAL9000.**: $SSBump + $Detail References from Portal 2
 - **Polydot**: Tester.
 - **Hexadot**: Tester.

---
### Third Party Code  
**LUX** uses Third-Party-Code, those are listed below.<br>
For more Details see **`thirdpartylegalnotices.txt`**  
 - **[Mapbase](https://github.com/mapbase-source/source-sdk-2013)**
 - **[Alien Swarm: Reactive Drop]([(https://github.com/ReactiveDrop/reactivedrop_public_src))**

---
## License

The **LUX** Codebase is licensed under the **[SOURCE 1 SDK LICENSE](LICENSE)**, which can be found in the LICENSE File at the root of this Repository.<br>
Proper Attribution must be included when using or redistributing any Part of the Codebase.<br>
The required Attribution is provided in **`thirdpartylegalnotices.txt`**, any additional Credits are always appreciated.

---

