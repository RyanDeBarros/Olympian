# Branches

## v9.1
* More editor work
	* Local per-document undo/redo stacks

## v9.2
* More editor work
	* Content Browser Panel
				
## v9.3
* More editor work
	* Project manager
		* Generate new project files
		* Recent project manifest

## v10
* Physics updates
* Utilities
	* TypewriterEffect
	* Dialog/decision trees
	* Push/pop text styles instead of embedded tags for code-based text input
	* FuzzySearch
	* Random class
* Coroutine system

## v11
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

## v12
* Font size caching/rounding + manual mipmap generation
* UI widget system
* Lighting/shadow/post-processing module
* AI: navigation, blackboard trees, etc.

## v13
* Separation of Tester project into separate repo
* Texture streaming
* Shader embedding
* Multithreading

## v14
* Networking

## v15
* Graphics API expansion/separation

# Misc

* Check `TODO LATER` for optimization/debt tasks.
* Check `TODO DEBT` for tech debt / maintenance tasks.

# Tech debt

* Add more support methods to classes to avoid chained access operators (e.g. `a.b.c->d.e()` shortened to `a.f()`)
* Move non-template inline methods to CPP files
* Use more forward declarations and move as many includes as possible to CPP files
* Create MKdocs for all public classes/functions
* Create asset loaders for all assets
* Create editor tabs for all assets
* Add more logging in engine and in editor
