# LUX for Source SDK 2013

**LUX** is a Shader Modification for the [Source Engine](https://developer.valvesoftware.com/wiki/Source).<br>
It serves as a drag & drop `materialsystem\` Replacement on the **[Source Engine SDK (2013)](https://developer.valvesoftware.com/wiki/Source_2013)**.<br>
<br>
Release Date: Q1 2026<br>

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
Now there are some Exceptions to this..<br>
The EnvMap Header is a bit large to accomodate Helper Functions and facilitate modifiability.<br> 
If you write new Shaders, splitting Features by Headers and Functions is suggested.<br>

</details>

<details>
<summary> Compatability with  Mapbase </summary>

**LUX** aims to be a viable Replacement for Mapbase Shaders.<br>
Because of this, specific Features have been added to accomodate it.<br>
In some Cases these Features cannot be used without also modifying Client.dll and/or Compilers.<br>
<br>
* Parallax Corrected Cubemaps require a custom VBSP.
* Treesway requires a custom Client.dll on SDK2013SP.
* Radial-Fog only controlable via ConVar on SDK2013SP.
<br>
To facilitate LUX working on Mapbase,<br>
every Shader has SDK_ Fallbacks that lead to the LUX_ Variants.<br>

</details>

<details>
<summary> Selective Stock Shader Replacement </summary>

Anyone who has touched the TF2SDK in recent Days,<br>
will have noticed that the VPC Setup isn't exactly great.<br>
To replace a Shader, it has to be added to the Project in VPC.<br>
This has two major Downsides.<br>
1. Need to regenerate the Solution File, everytime a new Shader is to be replaced.
2. Some Shaders use other Shaders as a dependency.<br>
This _sometimes_ makes it impossible to be selective.<br>
  
**LUX** solves both of these Issues.<br>
It provides a Headerfile with Defines that allow for the selective replacing of Shaders.-Among other Things.<br>
By Default all available Shaders are contained in the Project.<br>
Having to mess with the VPC Scripts to add an existing Shader is not necessary.<br>
<br>
**NOTE**:<br>
SDK2013SP does not allow for the replacement of Stock Shaders.<br>
Using Various Methods ( Memory Patching, Material Passthrough.. ),<br>
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
</details>

<details>
<summary> Documentation on Shaders and Parameters
</summary>

Due to the Development of **LUX**, Articles on the **VDC** such as `$lightwarptexture`, `$detail`, `UnlitTwoTexture` and `$envmapmask`
received massive updates, detailing them with information about caveats and bugs.<br>
**LUX** Shaders themselves are heavily commented, sometimes more than necessary..<br>
Structural Comments ( Section Separation ) have been added to every Shader to clearly differentiate the different Systems.<br>
Draw Functions have been organised to reflect this Change.<br>
Shaders now follow clearly defined Patterns/Layouts, this also makes it easier to create new Shaders.<br>
Instances where some whacky Behaviour from Stock-Shaders has to be considered to keep visual consistency, have been commented with "Stock-Consistency".<br>
<br>
A Github Wiki listing all Shader Parameters and all Shaders in the Projects is in the works.<br>
We added several Console Commands to help with this Endeavour.<br>
One of them allows dumping Information about Shaders in the .dll in a markdown ready Format.<br>
Shaders themselves can now List Details about the Geometry they are supposed to be applied to, Limitations and Performance which automatically gets added into the Markdown Document.<br>
<br>
Helper Strings have been added to *all* Parameters.<br>
They are massively more documented compared to their stock Variants.<br>
Texture Parameters detail their Channel-Usages, other Parameters define their Size and Type.<br>
Internal Shader Documentation can be looked at using the in-game Console as well.<br>

</details>

<details>
<summary> Features previously unavailable to Source 2013 </summary>

Various Features from other Branches have been implemented or recreated.<br>
This includes older Features as well, such as `$SelfIllumTexture` and `$EnvMapSphere`.<br>
Newer Features like `$PhongAlbedoBoost` (only in cs:go) have been approximated.<br>  
**NOTE**:
**LUX** is *not* made from leaked Code.<br>
Features (such as `$SSBumpMathFix`) have been derived from visual research and logical conclusions, using the Games the Features are from as a visual Reference.<br>
( Left 4 Dead 2, CS:GO, Portal 2 .. )

</details>

<details>
<summary> Fixing Bugs, Caveats, and missing Combos </summary>

**LUX** addresses *many* of the Issues Stock Shaders are known for.<br>
From straight up missing Features, to whacky stock Behaviour.<br>
The List of broken Things on Stock Shaders is so long,<br>
it's hard to even pick a representative Example.<br>

</details>

## Build instructions
Steps for building **LUX**:  
1. Run ```createallprojects.bat ``` to generate the `.sln` Solution File.
2. Compile the `client.dll`, `server.dll` and `game_shader_generic_example.dll`<br>
( `game_shader_dx9.dll` - SDK2013SP )
3. Run `src\materialsystem\stdshaders\$Compile_All.bat` to compile all the Shaders.
4. Copy the compiled Shaders found in `game\lux\shaders\fxc\` to your `mod\shaders\fxc\` Folder.<br>
And the .dll File from `game\lux\bin\` your `mod\bin\` Folder.<br>

The copying Process can be automated.<br>
To do so, the Destination Folder in the $Compile .bat Files and the VPC Scripts need to be adjusted.
<br>

---

Steps to implement **LUX** into an existing Project.<br>
1. Replace the `materialsystem` Folder in your `src\` with the one from LUX.
2. Follow Steps from `building LUX`

Quite simple, LUX is contained to the materialsystem Folder.<br>
Note that keeping commit-history of previous Files is not possible, due to how LUX was made.<br>
Previous Shader Files have been rewritten from Scratch and/or entirely removed. Including outdated <=DX8 Files.<br>

---

**NOTE (04.11.2025 DMY):**<br>
Currently a Constant Register Patch is required to avoid crashes.<br>
Due to Issues in the ( inaccesible ) ShaderAPI,<br>
* Launching the Game while replacing the UnlitGeneric and using any Constant Registers above c31, will crash the Game.<br>
* Closing the Game after using any Constant Register above c31, will crash the Game.<br>

Until this is fixed, the Constant Register Patch must be applied.<br>
For implementing this Patch, reference:<br>
* `src\game\client\cdll_client_int.cpp`
* `src\public\materialsystem\shaderapihack.h`

## Technical
For developing in the **LUX** Enviroment as a Programmer, Artist or both see the Information below.  
 - **[LUX Shader Wiki]()**  
 - **[LUX replaced Shaders Table]()**  
 - **[How to make a Shader compatible with LUX]()**  
 - **[Debugging Shaders]()**
 - **[Writing new Shaders on LUX]()**

## Additional Resources
* Vanilla `vbsp.exe` does not recognize custom Shaders.<br>
This causes `vrad.exe` to not compile Lightmaps for custom Shaders.<br> This is fixed in third party forks **[Mapbase](https://github.com/mapbase-source/source-sdk-2013)** and **[vbsp++](http://ficool2.github.io/HammerPlusPlus-Website/tools.html)**. 
* For Parallax corrected Cubemaps, `vbsp.exe` must be modified.<br>
See **[Parallax corrected Cubemaps](https://developer.valvesoftware.com/wiki/Parallax_Corrected_Cubemaps)**
 - **[Creating Shaders in the Source SDK 2013](https://developer.valvesoftware.com/wiki/Source_SDK_2013:_Shader_Authoring)**
 - **[LUX Valve Developer Wiki Page](https://developer.valvesoftware.com/wiki/LUX)**
 - **[LUX Github wiki]()**

## Contact
* Join the **[LUX Discord Server](https://discord.gg/qN7Ue3QA)**

## Legal
Any game or Modification that incorporates **LUX** or any Portion of its Codebase is required to include and redistribute the **`thirdpartylegalnotice.txt`** File.

---
### Contributing
Thank you for your interest in **LUX**!<br>
LUX is open to Contributions via the Pull-Requests System.<br>
By submitting a Pull-Request, you agree to comply with the **[SOURCE 1 SDK LICENSE](LICENSE)**<br>
Before making *any* Contribution, read the **[License Section](###license)**.

Proposed Contributions that violate the Terms of the **[SOURCE 1 SDK LICENSE](LICENSE)** will *not* be accepted.<br>
This includes, but is not limited to, using leaked or unauthorized use of Code.<br>
Additionally, if your Contribution includes third-party Code,<br>
you *must* provide Attribution in accordance with that Code’s License.<br>

---
### Credits  
Contributors refers to all Individuals or Entities that have submitted Code, Documentation, or other Materials to the **LUX** Project, during its Creation, either directly or indirectly.<br>
Each Contributors work is provided under the same License Terms as the **LUX** Project itself.<br>
Contributors are not legally liable or responsible for any Consequences, Damages, or Issues arising from the use, modification, or distribution of the **LUX** Codebase.<br>
Some **LUX** Contributors have chosen to remain anonymous; They are not listed below.<br>
If your contribution to the Project is not listed below, don't hesitate to get in contact.<br>

#### Major Contributors
 - **[ShiroDkxtro2](https://github.com/WhiteRedDragons)**: Creator of **LUX**.  
 - **[Trico Everfire](https://github.com/Trico-Everfire)**: Creation of Tools used during the Creation.  
 - **[Unusuario2](https://github.com/Unusuario2)** : GitHub README, Wiki, Shader Descriptions, and the ConCommand GitHub Wiki Dumper.   
 - **[azzy]()**: Code fixes.
 - **[InevitablyDivinity](https://github.com/InevitablyDivinity/)**: C++ Contributions, Linux Testing and general Source Engine-related Support.
 - **[Kerim 'Nbc66' Čamdžić](https://github.com/Nbc66)**: C++ Contributions, Github Setup.
 - **[Zeldaboy14](https://github.com/Zeldaboy14)**: Tester on **SDK2013SP** and **SDK2013MP**.

 #### Minor Contributors
 - **MrFunreal**: Provided his Documentation for the Infected Shader and Suggestions for **LUX** ($BlendModulateTransparency, $FlipNormal for the Refract Shader).
 - **MrKleiner**: Tools & Research.
 - **Krispy**: Tester on SDK2013MP before TF2SDK Update.
 - **[Jay]**: Tester.

---
### Third Party Code  
**LUX** uses Third-Party-Code, those are listed below.<br>
For more Details see **`thirdpartylegalnotice.txt`**  
 - **[Mapbase](https://github.com/mapbase-source/source-sdk-2013)**

---
### License

The **LUX** Codebase is licensed under the **[SOURCE 1 SDK LICENSE](LICENSE)**, which can be found in the LICENSE File at the root of this Repository.<br>
Proper Attribution must be included when using or redistributing any Part of the Codebase.<br>
The required Attribution is provided in **`thirdpartylegalnotice.txt`**, any additional Credits are always appreciated.

---

