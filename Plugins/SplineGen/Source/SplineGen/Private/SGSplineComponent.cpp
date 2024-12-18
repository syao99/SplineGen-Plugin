// Fill out your copyright notice in the Description page of Project Settings.


#include "SGSplineComponent.h"

// Sets default values for this component's properties
USGSplineComponent::USGSplineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USGSplineComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	UpdateUpVectorSpline(true);
	UpdateLocalOffsetPositionSpline();
	// ...
	
}


// Called every frame
void USGSplineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USGSplineComponent::UpdateSGSplines(bool bUpdateSplineFirst)
{
	UpdateUpVectorSpline(bUpdateSplineFirst);
	if (bEnableSmoothTangentsForLocalOffset) UpdateLocalOffsetPositionSpline(bUpdateSplineFirst);
}

void USGSplineComponent::UpdateUpVectorSpline(bool bUpdateSplineFirst)
{
	if (bUpdateSplineFirst) UpdateSpline();
	SplineCurveUpVector.Points.SetNum(SplineCurves.Rotation.Points.Num());
	for (int i = 0; i < SplineCurves.Rotation.Points.Num(); i++)
	{
		SplineCurveUpVector.Points[i] = FInterpCurvePoint(
			SplineCurves.Rotation.Points[i].InVal,
			SplineCurves.Rotation.Points[i].OutVal.RotateVector(FVector::UpVector),
			FVector::ZeroVector,
			FVector::ZeroVector,
			CIM_CurveAuto
		);
	}

	SplineCurveUpVector.bIsLooped = IsClosedLoop();
	// Ensure splines' looping status matches with that of the spline component
	if (IsClosedLoop())
	{
		const float LastKey = SplineCurveUpVector.Points.Num() > 0 ? SplineCurveUpVector.Points.Last().InVal : 0.0f;
		const float LoopKey = LastKey + 1.f;//bLoopPositionOverride ? LoopPosition : LastKey + 1.f;
		SplineCurveUpVector.SetLoopKey(LoopKey);
	}
	else
	{
		SplineCurveUpVector.ClearLoopKey();
	}

	// Automatically set the tangents on any CurveAuto keys
	SplineCurveUpVector.AutoSetTangents(0.0f, true);
}

void USGSplineComponent::UpdateLocalOffsetPositionSpline(bool bUpdateSplineFirst)
{
	//if (!bEnableSmoothTangentsForLocalOffset) return;
	if (bUpdateSplineFirst) UpdateSpline();
	int SegmentCount = FMath::RoundToInt(GetSplineLength() / TargetMeshLength);
	LocalOffsetSplineSegmentLength = GetSplineLength() / float(SegmentCount);
	SplineCurveLocalOffsetPosition.Points.SetNum(SegmentCount);

	for (int i = 0; i < SegmentCount; i++)
	{
		float CurrentDist = LocalOffsetSplineSegmentLength * i;
		SplineCurveLocalOffsetPosition.Points[i] = FInterpCurvePoint(
			float(i),
			GetLocalOffsetLocationAtDistanceAlongSpline(CurrentDist, LocalOffset, ESplineCoordinateSpace::Local),
			FVector::ZeroVector,
			FVector::ZeroVector,
			CIM_CurveAuto
		);
	}
	SplineCurveLocalOffsetPosition.bIsLooped = IsClosedLoop();
	if (IsClosedLoop())
	{
		const float LastKey = SplineCurveLocalOffsetPosition.Points.Num() > 0 ? SplineCurveLocalOffsetPosition.Points.Last().InVal : 0.0f;
		const float LoopKey = LastKey + 1.f;//bLoopPositionOverride ? LoopPosition : LastKey + 1.f;
		SplineCurveLocalOffsetPosition.SetLoopKey(LoopKey);
	}
	else
	{
		SplineCurveLocalOffsetPosition.ClearLoopKey();
	}

	SplineCurveLocalOffsetPosition.AutoSetTangents(0.0f, true);

	OffsetSplineEstimatedLength = GetSplineLength() + TargetMeshLength;//(LocalOffset.Length() * 4.f);
}

void USGSplineComponent::ApplyLocalOffset(FVector2D Offset)
{
	for (int i = 0; i < GetNumberOfSplinePoints(); i++)
	{
		ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;
		FVector NewLocation = GetLocationAtSplinePoint(i, LS) + (GetRightVectorAtSplinePoint(i, LS) * Offset.X) + (GetUpVectorAtSplinePoint(i, LS) * Offset.Y);
		SetLocationAtSplinePoint(i, NewLocation, LS, false);
	}
	UpdateSpline();
}

float USGSplineComponent::GetInputKeyAtLocationFromOffsetSpline(const FVector& InLocation, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	const FVector LocalLocation = (CoordinateSpace == ESplineCoordinateSpace::World) ? GetComponentTransform().InverseTransformPosition(InLocation) : InLocation;
	float Dummy;
	return SplineCurveLocalOffsetPosition.FindNearest(LocalLocation, Dummy);
}

FQuat USGSplineComponent::GetCorrectQuaternionAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	//return GetQuaternionAtSplineInputKey(InKey, CoordinateSpace);
	FQuat Quat = SplineCurves.Rotation.Eval(InKey, FQuat::Identity);
	Quat.Normalize();

	const FVector Direction = SplineCurves.Position.EvalDerivative(InKey, FVector::ZeroVector).GetSafeNormal();
	const FVector UpVector = SplineCurveUpVector.Eval(InKey, FVector::UpVector).GetSafeNormal();

	FQuat Rot = (FRotationMatrix::MakeFromXZ(Direction, UpVector)).ToQuat();

	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		Rot = GetComponentTransform().GetRotation() * Rot;
	}

	return Rot;
}

FQuat USGSplineComponent::GetCorrectQuaternionAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	const float Param = SplineCurves.ReparamTable.Eval(Distance, 0.0f);
	return GetCorrectQuaternionAtSplineInputKey(Param, CoordinateSpace);
}

FRotator USGSplineComponent::GetCorrectRotationAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	const float Param = SplineCurves.ReparamTable.Eval(Distance, 0.0f);
	return GetCorrectRotationAtSplineInputKey(Param, CoordinateSpace);
}

FRotator USGSplineComponent::GetCorrectRotationAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	return GetCorrectQuaternionAtSplineInputKey(InKey, CoordinateSpace).Rotator();
}

FVector USGSplineComponent::GetCorrectUpVectorAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	const FQuat Quat = GetCorrectQuaternionAtSplineInputKey(InKey, ESplineCoordinateSpace::Local);
	FVector UpVector = Quat.RotateVector(FVector::UpVector);

	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		UpVector = GetComponentTransform().TransformVectorNoScale(UpVector);
	}

	return UpVector;
	/*
	FVector Start = GetUpVectorAtSplinePoint(FMath::FloorToInt(InKey), ESplineCoordinateSpace::Local);
	FVector End = GetUpVectorAtSplinePoint(FMath::CeilToInt(InKey), ESplineCoordinateSpace::Local);
	float SubInputKey = InKey - FMath::FloorToFloat(InKey);
	//SubInputKey *= FMath::CubicInterp(Start, 0.f, End, 0.f, SubInputKey);
	FVector UpVector = FVector::SlerpVectorToDirection(Start, End, SubInputKey);
	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		UpVector = GetComponentTransform().TransformVectorNoScale(UpVector);
	}
	return UpVector;
	*/
}

FVector USGSplineComponent::GetCorrectUpVectorAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	const float Param = SplineCurves.ReparamTable.Eval(Distance, 0.0f);
	return GetCorrectUpVectorAtSplineInputKey(Param, CoordinateSpace);
}

FTransform USGSplineComponent::GetCorrectTransformAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace, bool bUseScale) const
{
	const FVector Location(GetLocationAtSplineInputKey(InKey, ESplineCoordinateSpace::Local));
	const FQuat Rotation(GetCorrectQuaternionAtSplineInputKey(InKey, ESplineCoordinateSpace::Local));
	const FVector Scale = bUseScale ? GetScaleAtSplineInputKey(InKey) : FVector(1.0f);

	FTransform Transform(Rotation, Location, Scale);

	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		Transform = Transform * GetComponentTransform();
	}

	return Transform;
}

FTransform USGSplineComponent::GetCorrectTransformAtDistanceAlongSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace, bool bUseScale) const
{
	const float Param = SplineCurves.ReparamTable.Eval(Distance, 0.0f);
	return GetCorrectTransformAtSplineInputKey(Param, CoordinateSpace, bUseScale);
}

int USGSplineComponent::GetLastSplinePoint() const
{
	return GetNumberOfSplinePoints() - 1;
}

int USGSplineComponent::GetLastSegment() const
{
	return GetNumberOfSplineSegments() - 1;
}

bool USGSplineComponent::IsSplinePointInRange(int SplinePoint, bool bUseSegments) const
{
	if (bUseSegments) (SplinePoint >= 0 && SplinePoint <= GetLastSegment());
	return (SplinePoint >= 0 && SplinePoint <= GetLastSplinePoint());
}

int USGSplineComponent::GetNextPreviousSplinePoint(int SplinePoint, bool IsNext, bool bUseSegments) const
{
	int Adder = IsNext ? 1 : -1;
	int Result = SplinePoint + Adder;
	if (IsSplinePointInRange(Result)) return Result;
	else if (Result > GetLastSplinePoint()) return 0;
	else if (Result < 0) return GetLastSplinePoint();
	else return -1;
}

int USGSplineComponent::GetNextSplinePoint(int SplinePoint, bool bUseSegments) const
{
	return GetNextPreviousSplinePoint(SplinePoint, true, bUseSegments);
}

int USGSplineComponent::GetPreviousSplinePoint(int SplinePoint, bool bUseSegments) const
{
	return GetNextPreviousSplinePoint(SplinePoint, false, bUseSegments);
}

int USGSplineComponent::GetNextPreviousInRange(int Value, int Min, int Max, bool IsNext) const
{
	int Adder = IsNext ? 1 : -1;
	int Result = Value + Adder;
	if (Result >= Min && Result <= Max) return Result;
	else if (Result < Min) return Max;
	else if (Result > Max) return Min;
	return 0;
}

int USGSplineComponent::WrapSplinePointToRange(int SplinePoint) const
{
	return FMath::Wrap(SplinePoint, 0, GetLastSplinePoint());
}

int USGSplineComponent::GetSegmentLength(int SplinePoint) const
{
	//SplinePoint = WrapSplinePointToRange(SplinePoint);
	//return SplineCurves.GetSegmentLength(SplinePoint, 1.f, IsClosedLoop(), GetComponentTransform().GetScale3D());
	
	float Start = GetDistanceAlongSplineAtSplinePoint(SplinePoint);
	int NextSplinePoint = GetNextSplinePoint(SplinePoint);
	float End = 0.f;
	if (NextSplinePoint != 0) End = GetDistanceAlongSplineAtSplinePoint(NextSplinePoint);
	else End = GetSplineLength();
	return FMath::Abs(End - Start);
}

FVector USGSplineComponent::GetLocalOffsetLocationAtSplineInputKey(float InKey, FVector2D NewLocalOffset, ESplineCoordinateSpace::Type CoordinateSpace)
{
	FVector Location = GetLocationAtSplineInputKey(InKey, CoordinateSpace);
	FVector LocalizedOffset3D = (GetRightVectorAtSplineInputKey(InKey, CoordinateSpace) * LocalOffset.X) + (GetCorrectUpVectorAtSplineInputKey(InKey, CoordinateSpace) * LocalOffset.Y);
	return Location + LocalizedOffset3D;
}

FVector USGSplineComponent::GetLocalOffsetLocationAtDistanceAlongSpline(float Distance, FVector2D NewLocalOffset, ESplineCoordinateSpace::Type CoordinateSpace)
{
	const float Param = SplineCurves.ReparamTable.Eval(Distance, 0.0f);
	return GetLocalOffsetLocationAtSplineInputKey(Param, LocalOffset, CoordinateSpace);
}

FVector USGSplineComponent::GetLocalOffsetLocationAtSplineInputKeyFromOffsetSpline(float InKey, ESplineCoordinateSpace::Type CoordinateSpace)
{
	FVector Location = SplineCurveLocalOffsetPosition.Eval(InKey, FVector::ZeroVector);
	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		Location = GetComponentTransform().TransformPosition(Location);
	}
	return Location;
}

FVector USGSplineComponent::GetLocalOffsetTangentAtSplineInputKeyFromOffsetSpline(float InKey, ESplineCoordinateSpace::Type CoordinateSpace)
{
	FVector Tangent = SplineCurveLocalOffsetPosition.EvalDerivative(InKey, FVector::ZeroVector);
	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		Tangent = GetComponentTransform().TransformVector(Tangent);
	}
	return Tangent;
}

FVector USGSplineComponent::GetLocalOffsetLocationAtDistanceAlongSplineFromOffsetSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace)
{
	//float Param = (Distance * (OffsetSplineEstimatedLength / GetSplineLength()) / LocalOffsetSplineSegmentLength);
	float Param = (Distance / LocalOffsetSplineSegmentLength);
	return GetLocalOffsetLocationAtSplineInputKeyFromOffsetSpline(Param, CoordinateSpace);
}

FVector USGSplineComponent::GetLocalOffsetTangentAtDistanceAlongSplineFromOffsetSpline(float Distance, ESplineCoordinateSpace::Type CoordinateSpace)
{
	//float Param = (Distance * (OffsetSplineEstimatedLength / GetSplineLength()) / LocalOffsetSplineSegmentLength);
	float Param = (Distance / LocalOffsetSplineSegmentLength);
	return GetLocalOffsetTangentAtSplineInputKeyFromOffsetSpline(Param, CoordinateSpace);
}