A GLSL Shader to Model Finished Wood
====================================

The original paper (by **Marschner et al.**) established a new sub-surface reflection component to the bidirectional reflectance distribution function (BRDF) that creates a sub-surface highlight distinct to the appearance of actual, finished wood. The key features of this process include identifying the direction/angle of the sub-surface wooden fibers, refracating and reflecting the incoming light ray along this angled wooden fiber, and producing a Gaussian-spread along a cone in the direction of this angled reflection ray. All of these features were coded and ported into GLSL. A sample, rendered image of tiger wood is presented below.

![sample wood shader on tiger wood](/presentation/img/tiger.png)


Shader Code
-----------

The BRDF has been coded into the wood shaders ( *wood-shader/wood.frag* & *wood-shader/wood.vert*). The main code for the BRDF is in *wood.frag*. It is based on the Renderman shader that the original authors used to produce their final images. The bulk of this project was understanding the Renderman code and writing the same shader from scratch in GLSL.

All the shader code matches the equations presented in **Section 4** of their paper, the *Shading Model for Wood*. This method specifically applies to the sub-surface reflection of finished wood, where light refracts and reflects off the internal wood fibers. The surface reflection, like in the original paper, has been implemented using a simple Phong model. Together, these two pieces (surface & sub-surface reflection) are the fundamental components of the BRDF.

Writing the shader code for the sub-surface reflection proved to be quite the challenge. Unlike other more robust shader languages, GLSL requires more manual work. For example, the fresnel function exists in the Renderman language, but that is not the case for GLSL. Reading [a white paper by Nvidia](http://http.download.nvidia.com/developer/SDK/Individual_Samples/DEMOS/Direct3D9/src/HLSL_FresnelReflection/docs/FresnelReflection.pdf), referenced within the code, the fresnel function was implemented, at least up until the fresnel factor. Then, the GLSL method refract() was used to alter the light rays (or vectors) accordingly. It took a lot of debugging and testing to get just the fresnel function working correctly.

Interestingly, the implementation of the shader used by the original authors for the rendered images in the paper had a mistake. I emailed the first author on the paper (Dr. Marschner at Cornell University), and pointed out an error in a piece of code they had used for the normalized Gaussian distribution. The problem was multiplying their Gaussian by the beta factor, instead of dividing. He stated in his email that the only real, resulting difference was a change in one of the other variables for generating the highlighted surface, but nonetheless, I have fixed this normalized Gaussian distribution in my shader implementation (*wood.frag*, line 152). I was not exactly sure which variable he said it would need to change, but that could explain some differences in the renders generated from my program.

![sample wood shader on walnut wood](/presentation/img/walnut-project.png)


Interface
---------

The GUI is the basic window and environment provided from a GLUT and GLUI tutorial, as seen in the image below (the textured wood is padauk here). The object is a simple, thin "slab" of wood. The object can be moved, deactivated, and textures can be changed. There is a choice between any (or all) of the following wood types: maple, padauk, walnut1, and walnut2. These textures and input data come straight from the original authors' published data, referenced below. By default, the wooden slab rotates about the scene, but this can be toggled off as well.

![wood shader gui](/writeup/gui.png)

The light and camera can both be moved to see the interactions with the object. Moving the light or rotating the camera will show how the sub-surface reflected highlight moves on the wooden surface, as can be seen in the images on this page. The keyboard button 'w' will toggle the wood shading model and textures off and on. A regular Phong shader is used to mimic a surface reflection highlight on the wood as well, as the original authors also did.

Additionally, several key variables in the BRDF have been added as adjustable variables in the GUI. First, the index of refraction can be adjusted, changing how the light travels through the wood sub-surface. Interestingly, this variable does not have a large change in the actual render output (from afar). The width of the sub-surface highlight can also be adjusted, which very obviously gets smaller or larger with the size of that variable. Lastly, the specular roughness is one of the factors used for the Gaussian distribution, so light gets more spread out along the surface as this value increases (though the change is not too large).


References
----------

the fragment shader for the wood ( *wood-shader/wood.frag*) and the whole idea behind this project is inspired from the paper below

[Measuring and Modeling the Appearance of Finished Wood](http://www.cs.cornell.edu/~srm/publications/SG05-wood.html)

**Stephen R. Marschner**, **Stephen H. Westin**, **Adam Arbree**, & **Jonathan T. Moon**

in Proceedings of *SIGGRAPH 2005*

all wood textures ( *wood-shader/tex*) are converted from the original authors' project

lastly, the fresnel refraction function is provided from [a white-paper by Nvidia](http://http.download.nvidia.com/developer/SDK/Individual_Samples/DEMOS/Direct3D9/src/HLSL_FresnelReflection/docs/FresnelReflection.pdf)
