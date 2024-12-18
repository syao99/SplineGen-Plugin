# Spline Gen Plugin

Crude, mostly undocumented plugin for resolving Unreal Engine's spline component twisting issues caused by incorrect quaternion cubic spline interpolation. The solution is simply create and use a spline of up vectors instead, which interpolate correctly and fix the spline twisting issue.

Plugin contains other various functionality for assisting runtime spline editing and spline mesh generation. Great for roller coasters and racetracks.

You will need a C++ project to build this plugin. See Unreal Engine's documentation on this - add a blank class to your project, add this plugin to your [Path/To/Your/Project]/Plugins directory, open your IDE, and build your project.

Built and tested in Unreal Engine 5.4. Samples/content will not work in older versions, code might.

## Classes:

### SGSplineComponent
Subclass of Unreal's SplineComponent. Contains functionality for corrected twisting issues. Contains a series of GetCorrected{Location,Rotation}At{DistanceAlongSpline,SplineInputKey} functions. See SGSplineComponent.h.

### SGMeshSplineComponent
Subclass of SGSplineComponent. Contains all sorts of helper functionality for implementing mesh splines and handling realtime update, including mesh and material updating, a point selection system, and isolated updates to just selected points. Functionality can be buggy and/or complex. Spline offset feature (commonly used for roller coaster heartlining) is not complete nor working properly, and for now you'd have to work around this by generating a separate SGSplineMeshComponent in Blueprint that's already heartlined. See Blueprint sample implementations included in the samples.

### SplineGenBPLibrary
Obsolete, flawed implementations, do not use. Included for reference purposes.

## PotentialUEModifications
This folder contains the UE source modifications I performed while experimenting with changes to resolve this issue at the engine level. The changes successfully allow splines to display correctly in the editor, but adding spline points and other common operations can cause crashes. This code is included **for reference only** and might be useful for Epic to implement a real engine fix, which would render a portion of this plugin obsolete.