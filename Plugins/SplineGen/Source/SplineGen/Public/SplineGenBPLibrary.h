// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
//#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "SGSplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "SG_Types.h"
#include "SplineGenBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class USplineGenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Get Next Spline Point"), Category = "SplineGen")
	static const int GetNextSplinePoint(const USplineComponent* Spline, const int SplinePoint = -1);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Get Previous Spline Point"), Category = "SplineGen")
	static const int GetPreviousSplinePoint(const USplineComponent* Spline, const int SplinePoint = -1);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Get Segment Length"), Category = "SplineGen")
	static const float GetSegmentLength(const USplineComponent* Spline, const int SplinePoint = 0);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Is Spline Point In Range"), Category = "SplineGen")
	static const bool IsSplinePointInRange(const USplineComponent* Spline, const int SplinePoint);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Get Spline Point"), Category = "SplineGen")
	static const FSplinePointSetting GetSplinePointData(const USplineComponent* Spline, const int SplinePointIndex);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Spline Point"), Category = "SplineGen")
	static void SetSplinePoint(USplineComponent* Spline, const int SplinePointIndex, const FSplinePointSetting SplinePointData, bool bUpdate);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Trim Spline"), Category = "SplineGen")
	static bool TrimSpline(USplineComponent* Spline, const int NewLastIndex);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Corrected Tangent Lengths"), Category = "SplineGen")
	static const FVector2D FindCorrectedTangentLengths(const USplineComponent* Spline, const int SplinePoint, float Offset);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Populate Spline if Point not in Range"), Category = "SplineGen")
	static int PopulateSplineIfPointNotInRange(USplineComponent* Spline, const int SplinePoint, const bool bUpdate = false);//, FVector Location, ESplineCoordinateSpace::Type CoordSpace);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Reference Spline"), Category = "SplineGen")
	static void UpdateRefSpline(const USplineComponent* UserSpline, USplineComponent* GenRefSpline, const int UserSplinePoint, const float RefSplineOffset = 10.f);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Calculate Mesh Spline Divisions From User"), Category = "SplineGen")
	static FMeshSplineDivisions CalcMeshSplineDivisionsFromUserSplineSegment(const USplineComponent* UserSpline, const USplineComponent* MeshSpline, const int UserSplinePoint, const float RequestSegmentLength = 100);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Mesh Spline User Segment"), Category = "SplineGen")
	static int UpdateMeshSplineSection(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, const int MeshSplineStartingPoint, const float TargetMeshSegmentLength, const bool bClearSpline, const bool bGenSplineDivisions, FMeshSplineDivisions SplineDivisions, UPARAM(ref) const FRollConfig& RollConfig);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Selected Ease Interp"), Category = "SplineGen")
	static const float SelectEaseInterp(const float A, const float B, const float Alpha, UPARAM(ref) const FRollConfig& RollConfig);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Mesh Spline Starting Point From User Point"), Category = "SplineGen")
	static const int FindMeshSplineStartingPointFromUserPoint(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int ControlPointOnUserSpline);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Spline Shift Points"), Category = "SplineGen")
	static void SplineShiftPoints(USplineComponent* Spline, int StartingPoint, int Shift, bool bUpdateSpline);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Mesh Spline"), Category = "SplineGen")
	static int UpdateMeshSpline(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, int MeshSplineStartPoint, const float TargetMeshSegmentLength, const int PreviousSegmentCount, const bool bClearMeshSpline, UPARAM(ref) const FRollConfig& RollConfig);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Mesh Spline Segment Count"), Category = "SplineGen")
	static int FindMeshSplineSegmentCount(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSegment = -1);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Nearest Mesh Spline Point From User Point"), Category = "SplineGen")
	static int FindNearestMeshSplinePointFromUserPoint(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSplinePoint = -1);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Init Ref Spline"), Category = "SplineGen")
	static void InitRefSpline(const USplineComponent* UserSpline, USplineComponent* GenRefSpline, const float RefSplineOffset);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Init Mesh Spline"), Category = "SplineGen")
	static void InitMeshSpline(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, const float TargetMeshSegmentLength, UPARAM(ref) const TArray<FRollConfig>& RollConfigs);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Set Mesh Spline Segment From User Spline Segment"), Category = "SplineGen")
	static int SetMeshSplineRegionFromUserSplineSegment(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, const float TargetMeshSegmentLength, UPARAM(ref) const TArray<FRollConfig>& RollConfigs);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Set Single Spline Mesh"), Category = "SplineGen")
	static void SetSingleSplineMesh(const USplineComponent* Spline, const int SplinePoint, USplineMeshComponent* SplineMesh, AActor* Actor, const bool bUpdate);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Init Spline Mesh"), Category = "SplineGen")
	static void InitSplineMesh(const USplineComponent* GenMeshSpline, UStaticMesh* Mesh, UPARAM(ref) TArray<UMaterialInterface*>& Materials, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Spline Mesh Region"), Category = "SplineGen")
	static void UpdateSplineMeshRegion(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSegment, const int Shift, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, UPARAM(ref) TArray<UMaterialInterface*>& Materials);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Style"), Category = "SplineGen")
	static void UpdateStyle(USplineMeshComponent* SplineMesh, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update Style Region"), Category = "SplineGen")
	static void UpdateStyleRegion(USplineComponent* UserSpline, int UserSegment, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, UStaticMesh* Mesh, UPARAM(ref) TArray<UMaterialInterface*>& Materials, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Clear All"), Category = "SplineGen")
	static void ClearAll(USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Init All"), Category = "SplineGen")
	static void InitAll(USplineComponent* UserSpline, USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, UPARAM(ref) TArray<UMaterialInterface*>& Materials, float RefSplineOffset, float TargetMeshSegmentLength, UPARAM(ref) const TArray<FRollConfig>& RollConfigs);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Update All"), Category = "SplineGen")
	static void UpdateAll(USplineComponent* UserSpline, int UserSplinePoint, USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, UPARAM(ref) TArray<int>& SplinePointMap, UPARAM(ref) TArray<int>& SegmentCountMap, UPARAM(ref) TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, UPARAM(ref) TArray<UMaterialInterface*>& Materials, float RefSplineOffset, float TargetMeshSegmentLength, UPARAM(ref) const TArray<FRollConfig>& RollConfigs);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Get Spline Meshes Of Region"), Category = "SplineGen")
	static TArray<USplineMeshComponent*> GetSplineMeshesOfRegion(int UserSegment, UPARAM(ref) TArray<int>& SplinePointMap, TArray<USplineMeshComponent*>& SplineMeshes);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Find Roll At Segment"), Category = "SplineGen")
	static float FindRollAtSegment(const USplineComponent* Spline, const int SplinePoint);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Get Roll Config"), Category = "SplineGen")
	static FRollConfig GetRollConfig(UPARAM(ref) const TArray<FRollConfig>& RollConfigs, int UserSplinePoint);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Set Spline Meshes Collision"), Category = "SplineGen")
	static void SetSplineMeshesCollision(TArray<USplineMeshComponent*>& SplineMeshes, ECollisionEnabled::Type NewType);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "SplineGen Find Roll Curve"), Category = "SplineGen")
	static float GenRollCurve(float Value, float Min, float Max, const float StartTangentLength, const float EndTangentLength, const float SegmentLength);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Corrected Roll At Distance Along Spline"), Category = "SplineGen")
	static float FindCorrectedRollAtDistanceAlongSpline(const USplineComponent* Spline, const float DistanceAlongSpline, const ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Corrected Roll At Input Key Along Spline"), Category = "SplineGen")
	static float FindCorrectedRollAtInputKeyAlongSpline(const USplineComponent* Spline, const float InputKey, const ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Corrected Quaternion At Distance Along Spline"), Category = "SplineGen")
	static FQuat FindCorrectedQuatAtDistanceAlongSpline(const USplineComponent* Spline, const float DistanceAlongSpline, const ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "SplineGen Find Corrected Quaternion At Input Key Along Spline"), Category = "SplineGen")
	static FQuat FindCorrectedQuatAtInputKeyAlongSpline(const USplineComponent* Spline, const float InputKey, const ESplineCoordinateSpace::Type CoordinateSpace);
};