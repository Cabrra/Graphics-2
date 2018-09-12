Nightmare Scene
===============
Graphics II DirectX class project from Full Sail University

## Topics 
	+ Discover how light & shadow enhance visual depth and realism.
	+ Learn about the new kinds of shaders available on modern hardware. 
	+ Build a unique 3D solo project to practice new found skills. 
	+ Apply powerful API based optimizations to get the most from your 3D hardware.
	+ Delve into advanced techniques used in the video game & simulation industries.

## Usage

This project was part of the Graphics-II course (Game Development degree) from Full Sail University. 
The repo includes the C++ code to run the shaders and visualize the effects in a multi-threaded environment using DirectX 10. The scene is composed by:

+ Drawing models loaded from file and in separate thread
+ 100 zombies that are getting out from the ground with textures and normal mapping placed randomly in the terrain
+ Trees with leaves, textures and normal mapping
+ Water generated using shaders and applied movement within the shader
+ A cube with dynamic shaders: Specular and Night vision effect)
+ A tower model with textures and normal mapping
+ Monster models with textures and normal mapping
+ Dynamically generated terrain using noise mapping from a image rgb data in a per-pixel manipulation using geometry
+ Proceduraly created indexed triangle primitive based geometry
+ Additional viewport (minimap placed in the uper right side)
+ Texture mapping -> a scene within a scene
+ Infinite skybox
+ Fog effect applied to the camera field of view
+ Applied functional per pixel directional light to drawn geometry
+ Applied functional per pixel point light to drawn geometry 
+ Applied functional per pixel spot light to drawn geometry
+ Real time rendering
+ Transparent geometry sorted by depth and drawn in correct order

The scene is fully traversable pressing the movement keys

+ Controls
	+ A/W/S/D - translate the camera Left/Up/Down/Right
	+ U/J - Translate camera forward/backwards
	+ Z/X - Rotate camera in the X axis
	+ C/V - Rotate camera in the Y axis
	+ NumPad 1 - change magic box effect (magic box is located in the upper right side of the island)
	+ Esc - close application

## Pictures

![Clown](https://raw.githubusercontent.com/Cabrra/cabrra.github.io/master/Images/directXII/clown.png?token=AI_RbR9FzgRq7jUBk5hlLzR0EJRIVLPXks5bofcIwA%3D%3D)

![Ground](https://github.com/Cabrra/cabrra.github.io/raw/master/Images/directXII/ground.png)

![Tower](https://github.com/Cabrra/cabrra.github.io/blob/master/Images/directXII/tower.png)

![tree](https://github.com/Cabrra/cabrra.github.io/blob/master/Images/directXII/tree.png)

![water](https://github.com/Cabrra/cabrra.github.io/blob/master/Images/directXII/water.png)

## Contributing

Please read [CONTRIBUTING.md](https://github.com/Cabrra/Contributing-template/blob/master/Contributing-template.md) for details on the code of conduct, and the process for submitting pull requests to me.

## Authors

* **Jagoba "Jake" Marcos** - [Cabrra](https://github.com/Cabrra)

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Full Sail University - Game Development Department
* Tim Turcich - Course Director
* L.Norri - Course Director
