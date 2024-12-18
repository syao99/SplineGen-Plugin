// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SGSplineComponent.generated.h"

/**
 * 
 */
UCLASS(Meta = (BlueprintSpawnableComponent))
class SPLINEGEN_API USGSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USGSplineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	float LocalOffsetSplineSegmentLength = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float OffsetSplineEstimatedLength = 0.f;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInterpCurveVector SplineCurveUpVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInterpCurveVector SplineCurveLocalOffsetPosition;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(InlineEditConditionToggle=true))
	bool bEnableLocalOffset = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bEnableLocalOffset"))
	FVector2D LocalOffset = FVector2D::ZeroVector;
	// get heartlined location on spline
	// get corrected heartlined tangents or rotation

	UPROPERTY(EditAnywhere)
	float TargetMeshLength = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bEnableLocalOffset"))
	bool bEnableSmoothTangentsForLocalOffset;

	UFUNCTION(BlueprintCallable, meta = (Keywords = "UpdateSGSplines"), Category = "")
	void UpdateSGSplines(bool bUpdateSplineFirst = false);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "UpdateUpVectorSpline"), Category = "")
	void UpdateUpVectorSpline(bool bUpdateSplineFirst = false);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "UpdateLocalOffsetPositionSpline"), Category = "")
	void UpdateLocalOffsetPositionSpline(bool bUpdateSplineFirst = false);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "UpdateUpVectorSpline"), Category = "")
	void ApplyLocalOffset(FVector2D Offset);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "GetDistanceAlongSplineAtLocationFromOffsetSpline"), Category = "")
	float GetInputKeyAtLocationFromOffsetSpline(const FVector& InLocation, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectQuaternionAtSplineInputKey"), Category = "")
	FQuat GetCorrectQuaternionAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectQuaternionAtDistanceAlongSpline"), Category = "")
	FQuat GetCorrectQuaternionAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectRotationAtDistanceAlongSpline"), Category = "")
	FRotator GetCorrectRotationAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectRotationAtSplineInputKey"), Category = "")
	FRotator GetCorrectRotationAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectUpVectorAtSplineInputKey"), Category = "")
	FVector GetCorrectUpVectorAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectUpVectorAtDistanceAlongSpline"), Category = "")
	FVector GetCorrectUpVectorAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectTransformAtSplineInputKey"), Category = "")
	FTransform GetCorrectTransformAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace, bool bUseScale) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetCorrectTransformAtDistanceAlongSpline"), Category = "")
	FTransform GetCorrectTransformAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace, bool bUseScale) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLastSplinePoint"), Category = "")
	int GetLastSplinePoint() const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLastSplinePoint"), Category = "")
	int GetLastSegment() const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "IsSplinePointInRange"), Category = "")
	bool IsSplinePointInRange(int SplinePoint, bool bUseSegments = false) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetNextPreviousSplinePoint"), Category = "")
	int GetNextPreviousSplinePoint(int SplinePoint, bool IsNext, bool bUseSegments = false) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetNextPreviousSplinePoint"), Category = "")
	int GetNextSplinePoint(int SplinePoint, bool bUseSegments = false) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetNextPreviousSplinePoint"), Category = "")
	int GetPreviousSplinePoint(int SplinePoint, bool bUseSegments = false) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetNextPreviousInRange"), Category = "")
	int GetNextPreviousInRange(int Value, int Min, int Max, bool IsNext) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "WrapSplinePointToRange"), Category = "")
	int WrapSplinePointToRange(int SplinePoint) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetSegmentLength"), Category = "")
	int GetSegmentLength(int SplinePoint) const;

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetLocationAtSplineInputKey"), Category = "")
	FVector GetLocalOffsetLocationAtSplineInputKey(float InKey, FVector2D NewLocalOffset, ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetLocationAtDistanceAlongSpline"), Category = "")
	FVector GetLocalOffsetLocationAtDistanceAlongSpline(float Distance, FVector2D NewLocalOffset, ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetLocationAtSplineInputKeyFromOffsetSpline"), Category = "")
	FVector GetLocalOffsetLocationAtSplineInputKeyFromOffsetSpline(float InKey, ESplineCoordinateSpace::Type CoordinateSpace);
	
	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetTangentAtSplineInputKeyFromOffsetSpline"), Category = "")
	FVector GetLocalOffsetTangentAtSplineInputKeyFromOffsetSpline(float InKey, ESplineCoordinateSpace::Type CoordinateSpace);

	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetLocationAtDistanceAlongSplineFromOffsetSpline"), Category = "")
	FVector GetLocalOffsetLocationAtDistanceAlongSplineFromOffsetSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace);
	
	UFUNCTION(BlueprintPure, meta = (Keywords = "GetLocalOffsetTangentAtDistanceAlongSplineFromOffsetSpline"), Category = "")
	FVector GetLocalOffsetTangentAtDistanceAlongSplineFromOffsetSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace);
};
