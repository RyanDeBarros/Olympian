# Branches

## v10
* Physics updates
* Clean up code in each class - consistent style, put methods in CPP, etc.

## v11
* More editor work
	* Project manager
		* Generate new project files
		* Recent project manifest

## v12
* Assets
	* Rigid body
	* Material
* Particle system updates
	* Lifetime updates
	* SDF shape textures
* Archetype
	* GameObject 
	* Lifetime methods
	* Scene graph

## v13
* Font size caching/rounding + manual mipmap generation
* Anti aliasing
* UI widget system
* Lighting/shadow/post-processing module
* AI: navigation, blackboard trees, etc.

## v14
* Separation of Tester project into separate repo
* Texture streaming
* Shader embedding
* Thread safety + multi-threading

# Misc

* Check `TODO LATER` for optimization/debt tasks.
* Check `TODO DEBT` for tech debt / maintenance tasks.

## Later

* Build tool that converts some assets to a binary format when copying assets to the output folder. This would be indicated by a meta field in the asset (editable by editor in advanced settings), and would be used to shrink very large text files into more efficient binary files. Would need to adapt any asset loaders that support it in meta to have a text-based loader and a binary loader.
* Network communication - online/local multiplayer.
* Graphics API expansion/separation
	* GL_NV_gpu_shader5 is only supported on NVIDIA GPUs. Add support for other GPUs.
	* Separate OpenGL/GLFW into independent module - so that different API backends can be implemented, like DirectX and Vulkan
* In editor, popup inside document window / content browser panel to view list of undo actions so you can click on a certain action to rollback/forward to
* Utilities
	* TypewriterEffect
	* Dialog / decision trees / state machine (custom markup language to define states, transitions, conditions - can link to them in code unless basic built-in conditions like trigger, bool, int comparison, etc.)
	* Push/pop text styles instead of embedded tags for code-based text input
	* FuzzySearch

## Tech debt

* Add more support methods to classes to avoid chained access operators (e.g. `a.b.c->d.e()` shortened to `a.f()`)
* Move non-template inline methods to CPP files
* Use more forward declarations and move as many includes as possible to CPP files
* Create MKdocs for all public classes/functions
* Create asset loaders for all assets
* Create editor tabs for all assets
* Add more logging in engine and in editor
