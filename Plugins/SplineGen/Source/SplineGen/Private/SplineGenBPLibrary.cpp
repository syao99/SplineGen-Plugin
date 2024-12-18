// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplineGenBPLibrary.h"
#include "SplineGen.h"

USplineGenBPLibrary::USplineGenBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

const int USplineGenBPLibrary::GetNextSplinePoint(const USplineComponent* Spline, const int SplinePoint)
{
	if (!Spline) return -1;
	if (SplinePoint >= 0 && SplinePoint < Spline->GetNumberOfSplinePoints() - 1) return SplinePoint + 1; // if spline point in range.
	if (SplinePoint == Spline->GetNumberOfSplinePoints() - 1 && Spline->IsClosedLoop()) return 0; // if spline point is last index.
	if (SplinePoint > Spline->GetNumberOfSplinePoints() - 1 || SplinePoint < 0) return -1; // if spline point is out of range.
	return -1;
}

const int USplineGenBPLibrary::GetPreviousSplinePoint(const USplineComponent* Spline, const int SplinePoint)
{
	if (!Spline) return -1;
	if (SplinePoint > 0 && SplinePoint <= Spline->GetNumberOfSplinePoints() - 1) return SplinePoint - 1; // if spline point in range.
	if (SplinePoint == 0 && Spline->IsClosedLoop()) return Spline->GetNumberOfSplinePoints() - 1; // if spline point is first index.
	if (SplinePoint > Spline->GetNumberOfSplinePoints() - 1 || SplinePoint < 0) return -2; // if spline point is out of range/
	return -2;
}

const float USplineGenBPLibrary::GetSegmentLength(const USplineComponent* Spline, const int SplinePoint)
{
	if (!Spline) return -1.f;
	float Start = Spline->GetDistanceAlongSplineAtSplinePoint(SplinePoint);
	float End = Spline->GetDistanceAlongSplineAtSplinePoint(GetNextSplinePoint(Spline, SplinePoint));
	float Length = FMath::Abs(End - Start);
	return Length;
}

const bool USplineGenBPLibrary::IsSplinePointInRange(const USplineComponent* Spline, const int SplinePoint)
{
	if (Spline) return SplinePoint >= 0 && SplinePoint < Spline->GetNumberOfSplinePoints();
	return false;
}

const FSplinePointSetting USplineGenBPLibrary::GetSplinePointData(const USplineComponent* Spline, const int SplinePointIndex)
{
	FSplinePointSetting SplinePoint;
	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;
	SplinePoint.LocationCoordSpace = WS;
	SplinePoint.Location = Spline->GetLocationAtSplinePoint(SplinePointIndex, WS);
	SplinePoint.UpVectorCoordSpace = WS;
	SplinePoint.UpVector = Spline->GetUpVectorAtSplinePoint(SplinePointIndex, WS);
	SplinePoint.TangentCoordSpace = LS;
	SplinePoint.Tangent = Spline->GetTangentAtSplinePoint(SplinePointIndex, LS);
	SplinePoint.Scale = Spline->GetScaleAtSplinePoint(SplinePointIndex);
	return SplinePoint;
}

void USplineGenBPLibrary::SetSplinePoint(USplineComponent* Spline, const int SplinePointIndex, const FSplinePointSetting SplinePointData, bool bUpdate)
{
	//PopulateSplineIfPointNotInRange(Spline, SplinePointIndex, true);
	while ((Spline->GetNumberOfSplinePoints() - 1) < SplinePointIndex)
	{
		Spline->AddSplinePoint(FVector::ZeroVector, SplinePointData.LocationCoordSpace.GetValue(), false);
	}
	Spline->SetLocationAtSplinePoint(SplinePointIndex, SplinePointData.Location, SplinePointData.LocationCoordSpace.GetValue(), false);
	Spline->SetUpVectorAtSplinePoint(SplinePointIndex, SplinePointData.UpVector, SplinePointData.UpVectorCoordSpace.GetValue(), false);
	Spline->SetTangentAtSplinePoint(SplinePointIndex, SplinePointData.Tangent, SplinePointData.TangentCoordSpace.GetValue(), false);
	Spline->SetScaleAtSplinePoint(SplinePointIndex, SplinePointData.Scale, false);
	if (bUpdate) Spline->UpdateSpline();
}

bool USplineGenBPLibrary::TrimSpline(USplineComponent* Spline, const int NewLastIndex)
{
	if (!Spline) return false;
	if (NewLastIndex >= Spline->GetNumberOfSplinePoints() || NewLastIndex < 0) return false;
	int RemoveCount = (Spline->GetNumberOfSplinePoints() - 1) - NewLastIndex;
	for (int i = 0; i < RemoveCount; i++)
	{
		Spline->RemoveSplinePoint(Spline->GetNumberOfSplinePoints() - 1);
	}
	return true;
}

const FVector2D USplineGenBPLibrary::FindCorrectedTangentLengths(const USplineComponent* Spline, const int SplinePoint, float Offset)
{
	if (!Spline || !IsSplinePointInRange(Spline, SplinePoint)) return FVector2D(-1.f);
	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;
	FVector2D NewTangentLengths = FVector2D(Spline->GetArriveTangentAtSplinePoint(SplinePoint, LS).Size(), Spline->GetLeaveTangentAtSplinePoint(SplinePoint, LS).Size());
	return NewTangentLengths + Offset;
}

int USplineGenBPLibrary::PopulateSplineIfPointNotInRange(USplineComponent* Spline, const int SplinePoint, const bool bUpdate)//, FVector Location, ESplineCoordinateSpace::Type CoordSpace)
{
	if (IsSplinePointInRange(Spline, SplinePoint)) return 0;
	UE_LOG(LogTemp, Display, TEXT("Spline point not in range, should be adding new."));
	for (int i = Spline->GetNumberOfSplinePoints(); i <= SplinePoint; i++)
	{ // Loop to add spline points to offset spline until in range.
		Spline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, false);
		UE_LOG(LogTemp, Display, TEXT("Added spline point at index: %d"), i);
	}
	UE_LOG(LogTemp, Display, TEXT("New spline point count on ref spline: %d"), Spline->GetNumberOfSplinePoints());
	if (bUpdate) Spline->UpdateSpline();
	return SplinePoint - Spline->GetNumberOfSplinePoints();
}

void USplineGenBPLibrary::UpdateRefSpline(const USplineComponent* UserSpline, USplineComponent* GenRefSpline, const int UserSplinePoint, const float RefSplineOffset)
{
	if (!UserSpline || !GenRefSpline) return;
	if (!IsSplinePointInRange(UserSpline, UserSplinePoint)) return;

	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;
	
	//UE_LOG(LogTemp, Display, TEXT("Begin --------------------------------------------------------------------------------------------"));

	//UE_LOG(LogTemp, Display, TEXT("Change user spline point: %i :: Spline point count on ref spline: %d"), UserSplinePoint, GenRefSpline->GetNumberOfSplinePoints());

	PopulateSplineIfPointNotInRange(GenRefSpline, UserSplinePoint, true);

	//UE_LOG(LogTemp, Display, TEXT("UserSplinePoint: %i"), UserSplinePoint);

	FVector UserPosition = UserSpline->GetLocationAtSplinePoint(UserSplinePoint, WS);
	FVector UserUpVector = UserSpline->GetUpVectorAtSplinePoint(UserSplinePoint, WS);
	FVector PositionOffset = UserUpVector * RefSplineOffset;
	FVector ArriveUserTangent = UserSpline->GetArriveTangentAtSplinePoint(UserSplinePoint, LS); //GetTangentAtSplinePoint(UserSplinePoint, LS);
	FVector LeaveUserTangent = UserSpline->GetLeaveTangentAtSplinePoint(UserSplinePoint, LS);

	//FVector2D NewTangentLengths = FindCorrectedTangentLengths(UserSpline, UserSplinePoint, Offset);
	//FVector CorrectedArriveTangent = ArriveUserTangent.GetSafeNormal() * NewTangentLengths.X;
	//FVector CorrectedLeaveTangent = LeaveUserTangent.GetSafeNormal() * NewTangentLengths.Y;

	// User spline point location with offset added
	GenRefSpline->SetSplinePointType(UserSplinePoint, UserSpline->GetSplinePointType(UserSplinePoint), false);

	GenRefSpline->SetLocationAtSplinePoint(UserSplinePoint, UserPosition + PositionOffset, WS, false);
	//UE_LOG(LogTemp, Display, TEXT("set location: UserSplinePoint: %i :: UserPosition: %f, %f, %f"), UserSplinePoint, UserPosition.X, UserPosition.Y, UserPosition.Z);

	GenRefSpline->SetUpVectorAtSplinePoint(UserSplinePoint, UserUpVector, WS, false);

	//GenRefSpline->SetTangentAtSplinePoint(UserSplinePoint, UserTangent, LS, false);
	GenRefSpline->SetTangentsAtSplinePoint(UserSplinePoint, ArriveUserTangent, LeaveUserTangent, LS, false);

	GenRefSpline->SetScaleAtSplinePoint(UserSplinePoint, UserSpline->GetScaleAtSplinePoint(UserSplinePoint), false);

	GenRefSpline->UpdateSpline();

	//UE_LOG(LogTemp, Display, TEXT("End --------------------------------------------------------------------------------------------"));
}

FMeshSplineDivisions USplineGenBPLibrary::CalcMeshSplineDivisionsFromUserSplineSegment(const USplineComponent* UserSpline, const USplineComponent* MeshSpline, const int UserSplinePoint, const float RequestSegmentLength)
{
	FMeshSplineDivisions SplineDivisions;
	SplineDivisions.UserSegmentLength = GetSegmentLength(UserSpline, UserSplinePoint);
	SplineDivisions.SegmentsCount = FMath::RoundToInt(SplineDivisions.UserSegmentLength / RequestSegmentLength);
	SplineDivisions.ResultSegmentLength = SplineDivisions.UserSegmentLength / SplineDivisions.SegmentsCount;
	return SplineDivisions;
}

int USplineGenBPLibrary::UpdateMeshSplineSection(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, const int MeshSplineStartingPoint, const float TargetMeshSegmentLength, const bool bClearSpline, const bool bGenSplineDivisions, FMeshSplineDivisions SplineDivisions, const FRollConfig& RollConfig)
{
	if (UserSplinePoint < 0) return 0;
	if (bGenSplineDivisions) SplineDivisions = CalcMeshSplineDivisionsFromUserSplineSegment(UserSpline, GenMeshSpline, UserSplinePoint, TargetMeshSegmentLength);
	if (bClearSpline) GenMeshSpline->ClearSplinePoints(true);

	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	float StartingDistanceOnUserSpline = UserSpline->GetDistanceAlongSplineAtSplinePoint(UserSplinePoint);

	//UE_LOG(LogTemp, Display, TEXT("UpdateMeshSplineSection Init Success. UserSplinePoint: %i, MeshSegmentLength: %f, StartingDistanceOnUserSpline: %f"), UserSplinePoint, SplineDivisions.ResultSegmentLength, StartingDistanceOnUserSpline);
	//UE_LOG(LogTemp, Display, TEXT("Update Section UserSplinePoint: %i, Divisions: %i"), UserSplinePoint, SplineDivisions.SegmentsCount);

	int MinPointsNeeded = MeshSplineStartingPoint + SplineDivisions.SegmentsCount;

	float UseDistanceOnSpline = 0.f;
	int CurrentPointOnMeshSpline = 0;
	FSplinePointSetting UseSplinePoint;
	for (int i = 0; i <= SplineDivisions.SegmentsCount; i++)
	{
		UseDistanceOnSpline = (i * SplineDivisions.ResultSegmentLength) + StartingDistanceOnUserSpline;

		//FVector LocationOnRefSpline = GenRefSpline->GetLocationAtDistanceAlongSpline(UseDistanceOnSpline, WS);
		/*FVector UpVectorStart = UserSpline->GetUpVectorAtSplinePoint(UserSplinePoint, WS).GetSafeNormal();
		FVector UpVectorEnd = UserSpline->GetUpVectorAtSplinePoint(GetNextSplinePoint(UserSpline, UserSplinePoint), WS).GetSafeNormal();
		float Key = float(i) / float(SplineDivisions.SegmentsCount);
		/*if (RollConfig.EaseType == EInterpInOutType::AutoEase)
		{
			float Incoming = UserSpline->GetLeaveTangentAtSplinePoint(UserSplinePoint, LS).Length();
			float Outgoing = UserSpline->GetArriveTangentAtSplinePoint(GetNextSplinePoint(UserSpline, UserSplinePoint), LS).Length();
			float AverageLength = (Incoming + Outgoing) * 0.5f;
			TRange<float> InRange = TRange<float>(0.f, 1000.f);
			TRange<float> OutRange = TRange<float>(0.f, 3.f);
			float NewEaseExp = FMath::GetMappedRangeValueClamped(InRange, OutRange, AverageLength);
			FRollConfig NewRollConfig = FRollConfig(EInterpInOutType::AutoEase, EInterpInOutSelection::EaseInOut, NewEaseExp);
			Key = SelectEaseInterp(0.f, 1.f, Key, NewRollConfig);
		}
		else Key = SelectEaseInterp(0.f, 1.f, Key, RollConfig);*/
		/*float StartTanLen = UserSpline->GetLeaveTangentAtSplinePoint(UserSplinePoint, ESplineCoordinateSpace::Local).Length();
		float EndTanLen = UserSpline->GetArriveTangentAtSplinePoint(GetNextSplinePoint(UserSpline, UserSplinePoint), ESplineCoordinateSpace::Local).Length();
		float SegLen = GetSegmentLength(UserSpline, UserSplinePoint);
		Key = GenRollCurve(Key, 0.f, 1.f, StartTanLen, EndTanLen, SegLen);
		*/
		UseSplinePoint.LocationCoordSpace = WS;
		UseSplinePoint.UpVectorCoordSpace = WS;
		UseSplinePoint.TangentCoordSpace = WS;
		UseSplinePoint.Location = UserSpline->GetLocationAtDistanceAlongSpline(UseDistanceOnSpline, WS);
		const USGSplineComponent* CastedUserSpline = Cast<USGSplineComponent>(UserSpline);
		if (CastedUserSpline) UseSplinePoint.UpVector = CastedUserSpline->GetCorrectUpVectorAtDistanceAlongSpline(UseDistanceOnSpline, WS);
		else UseSplinePoint.UpVector = UserSpline->GetUpVectorAtDistanceAlongSpline(UseDistanceOnSpline, WS);
		//UseSplinePoint.UpVector = FVector::SlerpVectorToDirection(UpVectorStart, UpVectorEnd, Key).GetSafeNormal();
		//UseSplinePoint.UpVector = UserSpline->GetUpVectorAtDistanceAlongSpline(UseDistanceOnSpline, WS);
		//UE_LOG(LogTemp, Display, TEXT("Key: %f"), Key);
		//UseSplinePoint.UpVector = (LocationOnRefSpline - UseSplinePoint.Location).GetSafeNormal();
		UseSplinePoint.Tangent = UserSpline->GetTangentAtDistanceAlongSpline(UseDistanceOnSpline, WS) / SplineDivisions.SegmentsCount;
		UseSplinePoint.Scale = UserSpline->GetScaleAtDistanceAlongSpline(UseDistanceOnSpline);
		CurrentPointOnMeshSpline = i + MeshSplineStartingPoint;
		SetSplinePoint(GenMeshSpline, CurrentPointOnMeshSpline, UseSplinePoint, true);
		//UE_LOG(LogTemp, Display, TEXT("Set Spline Point Current Point On Mesh Spline: %i"), CurrentPointOnMeshSpline);
		//UE_LOG(LogTemp, Display, TEXT("Add/Set Mesh Point %i @ Location: %s, UpVector: %s, Tangent: %s"), i, *UseSplinePoint.Location.ToString(), *UseSplinePoint.UpVector.ToString(), *UseSplinePoint.Tangent.ToString());
	}
	GenMeshSpline->UpdateSpline();
	return SplineDivisions.SegmentsCount;
}

const float USplineGenBPLibrary::SelectEaseInterp(const float A, const float B, const float Alpha, const FRollConfig& RollConfig)
{
	if (RollConfig.InOut == EInterpInOutSelection::None || RollConfig.EaseType == EInterpInOutType::Linear) return Alpha;
	switch (RollConfig.EaseType)
	{
		case EInterpInOutType::Circular:
			switch (RollConfig.InOut)
			{
			case EInterpInOutSelection::EaseIn:
				return FMath::InterpCircularIn(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseOut:
				return FMath::InterpCircularOut(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseInOut:
				return FMath::InterpCircularInOut(A, B, Alpha);
			break;
			}
		break;
		case EInterpInOutType::Ease:
			switch (RollConfig.InOut)
			{
			case EInterpInOutSelection::EaseIn:
				return FMath::InterpEaseIn(A, B, Alpha, RollConfig.EaseExp);
			break;
			case EInterpInOutSelection::EaseOut:
				return FMath::InterpEaseOut(A, B, Alpha, RollConfig.EaseExp);
			break;
			case EInterpInOutSelection::EaseInOut:
				return FMath::InterpEaseInOut(A, B, Alpha, RollConfig.EaseExp);
			break;
			}
		break;
		case EInterpInOutType::Expo:
			switch (RollConfig.InOut)
			{
			case EInterpInOutSelection::EaseIn:
				return FMath::InterpExpoIn(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseOut:
				return FMath::InterpExpoOut(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseInOut:
				return FMath::InterpExpoInOut(A, B, Alpha);
			break;
			}
		break;
		case EInterpInOutType::Sine:
			switch (RollConfig.InOut)
			{
			case EInterpInOutSelection::EaseIn:
				return FMath::InterpSinIn(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseOut:
				return FMath::InterpSinOut(A, B, Alpha);
			break;
			case EInterpInOutSelection::EaseInOut:
				return FMath::InterpSinInOut(A, B, Alpha);
			break;
			}
		break;
		case EInterpInOutType::AutoEase:
			return FMath::InterpEaseInOut(A, B, Alpha, RollConfig.EaseExp);
		break;
	}
	return Alpha;
}

const int USplineGenBPLibrary::FindMeshSplineStartingPointFromUserPoint(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int ControlPointOnUserSpline)
{
	if (!UserSpline || !GenMeshSpline) return -1;
	float DistAlongSpline = UserSpline->GetDistanceAlongSplineAtSplinePoint(ControlPointOnUserSpline);
	float NewInputKey = GenMeshSpline->GetInputKeyValueAtDistanceAlongSpline(DistAlongSpline);
	UE_LOG(LogTemp, Display, TEXT("DistAlongSpline: %f"), DistAlongSpline);
	UE_LOG(LogTemp, Display, TEXT("InputKeyAtDistAlongSpline: %f"), NewInputKey);
	int PointOnFinalSpline = FMath::RoundToInt(NewInputKey); //FMath::Floor(NewInputKey);
	UE_LOG(LogTemp, Display, TEXT("PointOnFinalSpline: %i"), PointOnFinalSpline);
	return PointOnFinalSpline;
}

void USplineGenBPLibrary::SplineShiftPoints(USplineComponent* Spline, int StartingPoint, int Shift, bool bUpdateSpline)
{
	if (!Spline) return;
	if (!IsSplinePointInRange(Spline, StartingPoint) || Shift == 0) return;
	int NextPoint = GetNextSplinePoint(Spline, StartingPoint);
	FVector LocationLS = Spline->GetLocationAtSplinePoint(StartingPoint, ESplineCoordinateSpace::Local);
	if (Shift > 0)
	{
		for (int i = 0; i < Shift; i++)
		{
			Spline->AddSplinePointAtIndex(FVector::ZeroVector, StartingPoint, ESplineCoordinateSpace::Local, bUpdateSpline);
		}
	}
	else if (Shift < 0)
	{
		int AbsShift = FMath::Abs(Shift);
		for (int i = 0; i < AbsShift; i++)
		{
			Spline->RemoveSplinePoint(StartingPoint, bUpdateSpline);
		}
	}
}

int USplineGenBPLibrary::UpdateMeshSpline(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, int MeshSplineStartPoint, const float TargetMeshSegmentLength, const int PreviousSegmentCount, const bool bClearMeshSpline, const FRollConfig& RollConfig)
{
	if (UserSplinePoint < 0) return -1;

	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	if (MeshSplineStartPoint <= -1) MeshSplineStartPoint = FindMeshSplineStartingPointFromUserPoint(UserSpline, GenMeshSpline, UserSplinePoint);
	if (bClearMeshSpline) GenMeshSpline->ClearSplinePoints(true);
	FMeshSplineDivisions SplineDivisions = CalcMeshSplineDivisionsFromUserSplineSegment(UserSpline, GenMeshSpline, UserSplinePoint, TargetMeshSegmentLength);
	if (PreviousSegmentCount > -1)
	{
		int Shift = SplineDivisions.SegmentsCount - PreviousSegmentCount;
		if (Shift != 0)
		{
			SplineShiftPoints(GenMeshSpline, MeshSplineStartPoint, Shift, true);
			//UE_LOG(LogTemp, Display, TEXT("Shift: %i on: %i"), Shift, UserSplinePoint);
		}
	}
	
	int SegmentCount = UpdateMeshSplineSection(UserSpline, GenRefSpline, GenMeshSpline, UserSplinePoint, MeshSplineStartPoint, TargetMeshSegmentLength, bClearMeshSpline, false, SplineDivisions, RollConfig);
	
	//float StartingDistanceOnUserSpline = UserSpline->GetDistanceAlongSplineAtSplinePoint(UserSplinePoint);
	
	//UE_LOG(LogTemp, Display, TEXT("UpdateMeshSpline Init Success. UserSplinePoint: %i, MeshSegmentLength: %f, StartingDistanceOnUserSpline: %f, Shift: %i"), UserSplinePoint, SplineDivisions.ResultSegmentLength, StartingDistanceOnUserSpline, Shift);
	
	return SegmentCount;
}

int USplineGenBPLibrary::FindMeshSplineSegmentCount(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSegment)
{
	int PointA = FindMeshSplineStartingPointFromUserPoint(UserSpline, GenMeshSpline, UserSegment);
	int PointB = FindMeshSplineStartingPointFromUserPoint(UserSpline, GenMeshSpline, GetNextSplinePoint(UserSpline, UserSegment));
	return FMath::Abs(PointB - PointA);
}

int USplineGenBPLibrary::FindNearestMeshSplinePointFromUserPoint(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSplinePoint)
{
	if (!UserSpline || !GenMeshSpline) return -1;
	if (UserSplinePoint < 0) return -1;
	FVector Location = UserSpline->GetLocationAtSplinePoint(UserSplinePoint, ESplineCoordinateSpace::World);
	return FMath::RoundToInt(GenMeshSpline->FindInputKeyClosestToWorldLocation(Location));
}

void USplineGenBPLibrary::InitRefSpline(const USplineComponent* UserSpline, USplineComponent* GenRefSpline, const float RefSplineOffset)
{
	GenRefSpline->ClearSplinePoints(true);
	for (int i = 0; i < UserSpline->GetNumberOfSplinePoints(); i++)
	{
		UpdateRefSpline(UserSpline, GenRefSpline, i, RefSplineOffset);
	}
}

void USplineGenBPLibrary::InitMeshSpline(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, const float TargetMeshSegmentLength, const TArray<FRollConfig>& RollConfigs)
{
	GenMeshSpline->ClearSplinePoints(true);
	SplinePointMap = { 0 };
	SegmentCountMap = {};
	int CurrentMeshSplineStartPoint = 0;
	int CurrentPreviousSegmentCount = 0;
	for (int i = 0; i < UserSpline->GetNumberOfSplineSegments(); i++)
	{
		CurrentPreviousSegmentCount = UpdateMeshSpline(UserSpline, GenRefSpline, GenMeshSpline, i, CurrentMeshSplineStartPoint, TargetMeshSegmentLength, CurrentPreviousSegmentCount, false, GetRollConfig(RollConfigs, i));
		CurrentMeshSplineStartPoint += CurrentPreviousSegmentCount;
		SegmentCountMap.Add(CurrentPreviousSegmentCount);
		SplinePointMap.Add(CurrentMeshSplineStartPoint);
		UE_LOG(LogTemp, Display, TEXT("Add to mesh spline: %i"), i);
	}
}

int USplineGenBPLibrary::SetMeshSplineRegionFromUserSplineSegment(const USplineComponent* UserSpline, const USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, const int UserSplinePoint, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, const float TargetMeshSegmentLength, const TArray<FRollConfig>& RollConfigs)
{
	if (!UserSpline || !GenRefSpline || !GenMeshSpline) return -1;
	if (!IsSplinePointInRange(UserSpline, UserSplinePoint)) return -1;
	if (!SplinePointMap.IsValidIndex(UserSplinePoint) || !SegmentCountMap.IsValidIndex(UserSplinePoint)) return -1;
	int MeshSplineStartPoint = SplinePointMap[UserSplinePoint];
	int PreviousSegmentCount = SegmentCountMap[UserSplinePoint];
	//UE_LOG(LogTemp, Display, TEXT("Array PassRef Initial Values SplinePointMap: %i, SegmentCountMap: %i"), MeshSplineStartPoint, PreviousSegmentCount);
	int NewSegmentCount = UpdateMeshSpline(UserSpline, GenRefSpline, GenMeshSpline, UserSplinePoint, MeshSplineStartPoint, TargetMeshSegmentLength, PreviousSegmentCount, false, GetRollConfig(RollConfigs, UserSplinePoint));
	int Difference = NewSegmentCount - PreviousSegmentCount;
	for (int i = UserSplinePoint + 1; i < SplinePointMap.Num(); i++)
	{
		SplinePointMap[i] += Difference;
		//UE_LOG(LogTemp, Display, TEXT("Array PassRef Update SplinePointMap Index: %i, Value: %i"), i, SplinePointMap[i]);
	}
	SegmentCountMap[UserSplinePoint] += Difference;
	//UE_LOG(LogTemp, Display, TEXT("Array PassRef Update SegmentCountMap: %i"), SegmentCountMap[UserSplinePoint]);
	return Difference;
}

void USplineGenBPLibrary::SetSingleSplineMesh(const USplineComponent* Spline, const int SplinePoint, USplineMeshComponent* SplineMesh, AActor* Actor, const bool bUpdate)
{
	if (!Spline || !SplineMesh) return;
	if (!IsSplinePointInRange(Spline, SplinePoint)) return;

	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	int NextSplinePoint = GetNextSplinePoint(Spline, SplinePoint);
	SplineMesh->SetStartAndEnd(
		Spline->GetLocationAtSplinePoint(SplinePoint, LS),
		Spline->GetTangentAtSplinePoint(SplinePoint, LS),
		Spline->GetLocationAtSplinePoint(NextSplinePoint, LS),
		Spline->GetTangentAtSplinePoint(NextSplinePoint, LS),
		false
	);
	FVector2D StartScale = FVector2D(Spline->GetScaleAtSplinePoint(SplinePoint).Y, Spline->GetScaleAtSplinePoint(SplinePoint).Z);
	FVector2D EndScale = FVector2D(Spline->GetScaleAtSplinePoint(NextSplinePoint).Y, Spline->GetScaleAtSplinePoint(NextSplinePoint).Z);
	SplineMesh->SetStartScale(StartScale, false);
	SplineMesh->SetEndScale(EndScale, false);
	SplineMesh->SetSplineUpDir(Spline->GetUpVectorAtSplinePoint(SplinePoint, LS), true);

	float EndRoll = FindRollAtSegment(Spline, SplinePoint);
	SplineMesh->SetStartRollDegrees(0.f, false);
	SplineMesh->SetEndRollDegrees(EndRoll, false);
	//UE_LOG(LogTemp, Display, TEXT("Use End Roll Val: %f"), EndRoll);
	if (bUpdate) SplineMesh->UpdateMesh();
}

void USplineGenBPLibrary::InitSplineMesh(const USplineComponent* GenMeshSpline, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials, TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor)
{
	if (!Actor || !Mesh || !Materials.IsValidIndex(0)) return;
	for (int i = 0; i < SplineMeshes.Num(); i++) if (SplineMeshes[i]) SplineMeshes[i]->DestroyComponent(false);
	SplineMeshes.Empty();
	for (int i = 0; i < GenMeshSpline->GetNumberOfSplineSegments(); i++)
	{
		FString ComponentName = "TrackSplineMeshComponent" + FString::FromInt(0) + "_" + FString::FromInt(i);
		USplineMeshComponent* NewMesh = NewObject<USplineMeshComponent>(Actor, USplineMeshComponent::StaticClass(), *ComponentName);//Actor->CreateDefaultSubobject<USplineMeshComponent>(*ComponentName);
		// UNIQUE NAME REQUIRED, DO NOT USE - USplineMeshComponent* NewMesh = NewObject<USplineMeshComponent>(Actor, USplineMeshComponent::StaticClass(), TEXT("TrackSplineMeshComponent"));
		if (NewMesh)
		{
			NewMesh->RegisterComponent();
			NewMesh->SetMobility(EComponentMobility::Movable);
			NewMesh->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
			NewMesh->SetStaticMesh(Mesh);
			for (int j = 0; j < Materials.Num(); j++)
			{
				NewMesh->SetMaterial(j, Materials[j]);
			}
			SetSingleSplineMesh(GenMeshSpline, i, NewMesh, Actor, true);
			SplineMeshes.Add(NewMesh);
		}
	}
}

void USplineGenBPLibrary::UpdateSplineMeshRegion(const USplineComponent* UserSpline, const USplineComponent* GenMeshSpline, const int UserSegment, const int Shift, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, UPARAM(ref)TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials)//segment map, spline point map, spline meshes, user segment
{
	if (!UserSpline || !GenMeshSpline) return;
	if (!IsSplinePointInRange(UserSpline, UserSegment)) return;

	//1. checks, 2. change existing, 3. remove if fewer, 4. add if more, 5. ??
	int SegmentCount = -1;
	UE_LOG(LogTemp, Display, TEXT("Update Spline Mesh Region User Segment: %i"), UserSegment);
	if (!SegmentCountMap.IsValidIndex(UserSegment) || !SplinePointMap.IsValidIndex(UserSegment)) return;
	//if (/*!SegmentCountMap.IsValidIndex(GetNextSplinePoint(UserSpline,UserSegment)) || */!SplinePointMap.IsValidIndex(GetNextSplinePoint(UserSpline,UserSegment))) return;
	SegmentCount = SegmentCountMap[UserSegment];
	UE_LOG(LogTemp, Display, TEXT("Update Spline Mesh Region SegmentCount: %i"), SegmentCount);
	int StartMeshSplinePoint = -1;
	int EndMeshSplinePoint = -1;
	int LastIndex = UserSpline->GetNumberOfSplinePoints() - 1;
	StartMeshSplinePoint = SplinePointMap[UserSegment];
	EndMeshSplinePoint = SplinePointMap[GetNextSplinePoint(UserSpline,UserSegment)];
	int UseEndMeshSplinePoint = GetPreviousSplinePoint(GenMeshSpline, EndMeshSplinePoint);
	UE_LOG(LogTemp, Display, TEXT("StartMeshSplinePoint: %i; EndMeshSplinePoint: %i; UseEndMeshSplinePoint: %i"), StartMeshSplinePoint, EndMeshSplinePoint, UseEndMeshSplinePoint);
	if (Shift > 0)
	{
		for (int i = 0; i < Shift; i++)
		{
			if (SplineMeshes.IsValidIndex(UseEndMeshSplinePoint))
			{
				SplineMeshes.EmplaceAt(UseEndMeshSplinePoint);
			}
			else
			{
				SplineMeshes.Add(nullptr);
				UE_LOG(LogTemp, Display, TEXT("weird condition: shift > 0, needed to add nullptr to end of SplineMeshes[] array."));
			}
		}
	}
	else if (Shift < 0)
	{
		int AbsShift = FMath::Abs(Shift);
		for (int i = 0; i < AbsShift; i++)
		{
			int Counter = UseEndMeshSplinePoint - i;
			if (SplineMeshes.IsValidIndex(Counter))
			{
				if (SplineMeshes[Counter])
				{
					SplineMeshes[Counter]->DestroyComponent(false);
				}
				else UE_LOG(LogTemp, Display, TEXT("bad condition: shift < 0, SplineMeshes[] array has valid index, but no valid corresponding SPMesh Component."));
				SplineMeshes.RemoveAt(Counter);
			}
			else UE_LOG(LogTemp, Display, TEXT("bad condition: shift < 0, attempting to remove invalid index for SplineMeshes[] array."));
		}
	}

	int CurrentMeshSplinePoint = 0;
	for (int i = StartMeshSplinePoint; i <= UseEndMeshSplinePoint; i++)
	{
		if (!SplineMeshes.IsValidIndex(i)) SplineMeshes.EmplaceAt(i);
		if (!SplineMeshes[i])
		{
			//FString ComponentName = "TrackSplineMeshComponent" + FString::FromInt(UserSegment) + "_" + FString::FromInt(i) + FString::FromInt(FMath::RandRange(0,1000000)) + "U";
			FName NewComponentName = MakeUniqueObjectName(Actor, USplineMeshComponent::StaticClass(), FName("TrackSplineMeshComponent"));
			USplineMeshComponent* NewMesh = NewObject<USplineMeshComponent>(Actor, USplineMeshComponent::StaticClass(), NewComponentName);//*ComponentName
			if (NewMesh)
			{
				NewMesh->RegisterComponent();
				NewMesh->SetMobility(EComponentMobility::Movable);
				NewMesh->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
				if (!Mesh) NewMesh->SetStaticMesh(SplineMeshes[0]->GetStaticMesh());
				else NewMesh->SetStaticMesh(Mesh);
				if (Materials.IsValidIndex(0))
				{
					for (int j = 0; j < Materials.Num(); j++)
					{
						NewMesh->SetMaterial(j, Materials[j]);
					}
				}
				SplineMeshes[i] = NewMesh;
			}
			else UE_LOG(LogTemp, Display, TEXT("bad condition: no valid NewMesh"));
		}
		SetSingleSplineMesh(GenMeshSpline, i, SplineMeshes[i], Actor, true);
		//UE_LOG(LogTemp, Display, TEXT("Set Single Spline Mesh Index: %i"), i);
		//if (Shift != 0) UE_LOG(LogTemp, Display, TEXT("Set Single Mesh Calc'd Difference: %i, Shift: %i"),EndMeshSplinePoint-StartMeshSplinePoint,Shift);
	}
}

/*

SplinePointMap: indices: spline point ID's on user spline. values: starting spline point ID's on mesh spline per user spline point.
SegmentCountMap: indices: spline point ID's on user spline. values: number of spline points on mesh spline per user spline point.

*/

void USplineGenBPLibrary::UpdateStyle(USplineMeshComponent* SplineMesh, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials)
{
	if (!SplineMesh) return;
	if (Mesh) SplineMesh->SetStaticMesh(Mesh);
	//if (Materials.Num() > 0) SplineMesh->SetMaterial(0, Material);
	for (int i = 0; i < Materials.Num(); i++)
	{
		SplineMesh->SetMaterial(i, Materials[i]);
	}
}

void USplineGenBPLibrary::UpdateStyleRegion(USplineComponent* UserSpline, int UserSegment, TArray<USplineMeshComponent*>& SplineMeshes, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap)
{
	//TArray<USplineMeshComponent*> SplineMeshesRegion = GetSplineMeshesOfRegion(UserSegment, SplinePointMap, SplineMeshes);
	for (int i = 0; i < SegmentCountMap[UserSegment]; i++)
	{
		UpdateStyle(SplineMeshes[SplinePointMap[UserSegment]+i], Mesh, Materials);
	}
}

void USplineGenBPLibrary::ClearAll(USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor)
{
	if (GenRefSpline) GenRefSpline->ClearSplinePoints();
	if (GenMeshSpline) GenMeshSpline->ClearSplinePoints();

	USplineMeshComponent* CurrentComponent = Actor->GetComponentByClass<USplineMeshComponent>();
	while (CurrentComponent)
	{
		CurrentComponent->DestroyComponent();
		CurrentComponent = Actor->GetComponentByClass<USplineMeshComponent>();
	}

	SplinePointMap.Empty();
	SegmentCountMap.Empty();
	for (int i = 0; i < SplineMeshes.Num(); i++) if (SplineMeshes[i]) SplineMeshes[i]->DestroyComponent(false);
	SplineMeshes.Empty();
}

void USplineGenBPLibrary::InitAll(USplineComponent* UserSpline, USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials, float RefSplineOffset, float TargetMeshSegmentLength, const TArray<FRollConfig>& RollConfigs)
{
	ClearAll(GenRefSpline, GenMeshSpline, SplinePointMap, SegmentCountMap, SplineMeshes, Actor);
	//InitRefSpline(UserSpline, GenRefSpline, RefSplineOffset);
	InitMeshSpline(UserSpline, GenRefSpline, GenMeshSpline, SplinePointMap, SegmentCountMap, TargetMeshSegmentLength, RollConfigs);
	InitSplineMesh(GenMeshSpline, Mesh, Materials, SplineMeshes, Actor);
}

void USplineGenBPLibrary::UpdateAll(USplineComponent* UserSpline, int UserSplinePoint, USplineComponent* GenRefSpline, USplineComponent* GenMeshSpline, TArray<int>& SplinePointMap, TArray<int>& SegmentCountMap, TArray<USplineMeshComponent*>& SplineMeshes, AActor* Actor, UStaticMesh* Mesh, TArray<UMaterialInterface*>& Materials, float RefSplineOffset, float TargetMeshSegmentLength, const TArray<FRollConfig>& RollConfigs)
{
	//UpdateRefSpline(UserSpline, GenRefSpline, UserSplinePoint, RefSplineOffset);
	int CurrentShift;
	int PreviousPoint = GetPreviousSplinePoint(UserSpline, UserSplinePoint);
	int LastIndex = UserSpline->GetNumberOfSplinePoints() - 1;
	UE_LOG(LogTemp, Display, TEXT("try update: %i"), UserSplinePoint);
	//UE_LOG(LogTemp, Display, TEXT("check1: %s"), (UserSpline->IsClosedLoop() || UserSplinePoint > 0)? TEXT("true") : TEXT("false"));
	//if ((UserSpline->IsClosedLoop() || UserSplinePoint > 0) && IsSplinePointInRange(UserSpline, PreviousPoint))
	//if (true)//IsSplinePointInRange(UserSpline, PreviousPoint))
	
		UE_LOG(LogTemp, Display, TEXT("Tick Update Previous UserSegment %i"), PreviousPoint);
		CurrentShift = SetMeshSplineRegionFromUserSplineSegment(UserSpline, GenRefSpline, GenMeshSpline, PreviousPoint, SplinePointMap, SegmentCountMap, TargetMeshSegmentLength, RollConfigs);
		UpdateSplineMeshRegion(UserSpline, GenMeshSpline, PreviousPoint, CurrentShift, SplinePointMap, SegmentCountMap, SplineMeshes, Actor, Mesh, Materials);
	
	//UE_LOG(LogTemp, Display, TEXT("check1b: %s"), (UserSpline->IsClosedLoop() || UserSplinePoint < LastIndex)? TEXT("true") : TEXT("false"));
	//if ((UserSpline->IsClosedLoop() || UserSplinePoint <= LastIndex) && IsSplinePointInRange(UserSpline, UserSplinePoint))
	//if (true)//IsSplinePointInRange(UserSpline, UserSplinePoint))
	
		UE_LOG(LogTemp, Display, TEXT("Tick Update Current UserSegment %i"), UserSplinePoint);
		CurrentShift = SetMeshSplineRegionFromUserSplineSegment(UserSpline, GenRefSpline, GenMeshSpline, UserSplinePoint, SplinePointMap, SegmentCountMap, TargetMeshSegmentLength, RollConfigs);
		UpdateSplineMeshRegion(UserSpline, GenMeshSpline, UserSplinePoint, CurrentShift, SplinePointMap, SegmentCountMap, SplineMeshes, Actor, Mesh, Materials);
	
}

TArray<USplineMeshComponent*> USplineGenBPLibrary::GetSplineMeshesOfRegion(int UserSegment, TArray<int>& SplinePointMap, TArray<USplineMeshComponent*>& SplineMeshes)
{
	TArray<USplineMeshComponent*> SplineMeshesRegion;
	for (int i = SplinePointMap[UserSegment]; i < SplinePointMap[UserSegment+1] - 1; i++)
	{
		SplineMeshesRegion.Add(SplineMeshes[i]);
	}
	return SplineMeshesRegion;
}

float USplineGenBPLibrary::FindRollAtSegment(const USplineComponent* Spline, const int SplinePoint)
{
	if (!Spline) return 0.f;
	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	int NextSplinePoint = GetNextSplinePoint(Spline, SplinePoint);

	FVector TangentNext = Spline->GetTangentAtSplinePoint(NextSplinePoint, WS);
	FVector CrossProductCurrent = FVector::CrossProduct(TangentNext.GetSafeNormal(), Spline->GetUpVectorAtSplinePoint(SplinePoint, WS));
	FVector CrossProductNext = FVector::CrossProduct(TangentNext.GetSafeNormal(), Spline->GetUpVectorAtSplinePoint(NextSplinePoint, WS));
	FVector CrossProductBoth = FVector::CrossProduct(CrossProductCurrent, CrossProductNext).GetSafeNormal();
	float DotProductA = FVector::DotProduct(CrossProductCurrent.GetSafeNormal(), CrossProductNext.GetSafeNormal());
	float DotProductB = FVector::DotProduct(CrossProductBoth.GetSafeNormal(), TangentNext);
	return FMath::RadiansToDegrees(UKismetMathLibrary::SignOfFloat(DotProductB) * -1 * FGenericPlatformMath::Acos(DotProductA));
}

FRollConfig USplineGenBPLibrary::GetRollConfig(const TArray<FRollConfig>& RollConfigs, int UserSplinePoint)
{
	if (RollConfigs.IsValidIndex(UserSplinePoint)) return RollConfigs[UserSplinePoint];
	int LastIndex = RollConfigs.Num() - 1;
	if (LastIndex >= 0) return RollConfigs[LastIndex];
	else return FRollConfig();
}

void USplineGenBPLibrary::SetSplineMeshesCollision(TArray<USplineMeshComponent*>& SplineMeshes, ECollisionEnabled::Type NewType)
{
	for (int i = 0; i < SplineMeshes.Num(); i++)
	{
		SplineMeshes[i]->SetCollisionEnabled(NewType);
	}
}

float USplineGenBPLibrary::GenRollCurve(float Value, float Min, float Max, const float StartTangentLength, const float EndTangentLength, const float SegmentLength)
{
	//float StartTanVal = FMath::Clamp((StartTangentLength / SegmentLength), 0.f, 1.f * 0.5f);
	//float EndTanVal = FMath::Clamp((EndTangentLength / SegmentLength), 0.f, 1.f * 0.5f);
	float StartTanVal = StartTangentLength / SegmentLength;
	float EndTanVal = EndTangentLength / SegmentLength;
	//Value = FMath::Clamp(Value, Min, Max);
	//TArray<FVector2D> LookupTable;
	float CurrentTVal = 0.f;
	//float TValStep = Max / float(Resolution);
	FVector2D P1 = FVector2D(0.f, 0.f);
	FVector2D P2 = FVector2D(StartTanVal, 0.f);
	FVector2D P3 = FVector2D(EndTanVal, 1.f);
	FVector2D P4 = FVector2D(1.f, 1.f);
	/*FVector2D LP1;
	FVector2D LP2;
	FVector2D LP3;
	FVector2D FP1;
	FVector2D FP2;*/
	//for (int i = 0; i < Resolution; i++)
	//{
		//CurrentTVal = TValStep * float(i);
	CurrentTVal = Value;
	FVector2D LP1 = FMath::Lerp(P1, P2, CurrentTVal);
	FVector2D LP2 = FMath::Lerp(P2, P3, CurrentTVal);
	FVector2D LP3 = FMath::Lerp(P3, P4, CurrentTVal);
	FVector2D FP1 = FMath::Lerp(LP1, LP2, CurrentTVal);
	FVector2D FP2 = FMath::Lerp(LP2, LP3, CurrentTVal);
	return FMath::Lerp(FP1, FP2, CurrentTVal).Y;
		//LookupTable.Add(FMath::Lerp(FP1, FP2, CurrentTVal));
	//}
	//return LookupTable;
}

float USplineGenBPLibrary::FindCorrectedRollAtDistanceAlongSpline(const USplineComponent* Spline, const float DistanceAlongSpline, const ESplineCoordinateSpace::Type CoordinateSpace)
{
	return FindCorrectedRollAtInputKeyAlongSpline(Spline, Spline->GetInputKeyAtDistanceAlongSpline(DistanceAlongSpline), CoordinateSpace);
}

float USplineGenBPLibrary::FindCorrectedRollAtInputKeyAlongSpline(const USplineComponent* Spline, const float InputKey, const ESplineCoordinateSpace::Type CoordinateSpace)
{
	float InputKeyFloor = FMath::Floor(InputKey);
	float LocalKey = InputKey - InputKeyFloor;
	int SplinePoint = int(InputKeyFloor);
	float StartTanLen = Spline->GetLeaveTangentAtSplinePoint(SplinePoint, ESplineCoordinateSpace::Local).Length();
	float EndTanLen = Spline->GetArriveTangentAtSplinePoint(GetNextSplinePoint(Spline, SplinePoint), ESplineCoordinateSpace::Local).Length();
	float SegLen = GetSegmentLength(Spline, SplinePoint);
	LocalKey = GenRollCurve(LocalKey, 0.f, 1.f, StartTanLen, EndTanLen, SegLen);
	float NewKey = InputKeyFloor + LocalKey;
	FQuat StartQuat = Spline->GetQuaternionAtSplinePoint(SplinePoint, ESplineCoordinateSpace::Local);
	FQuat EndQuat = Spline->GetQuaternionAtSplinePoint(GetNextSplinePoint(Spline, SplinePoint), ESplineCoordinateSpace::Local);
	FQuat InterpedQuat = FQuat::Slerp(StartQuat, EndQuat, LocalKey);
	return InterpedQuat.W;
}

FQuat USplineGenBPLibrary::FindCorrectedQuatAtDistanceAlongSpline(const USplineComponent* Spline, const float DistanceAlongSpline, const ESplineCoordinateSpace::Type CoordinateSpace)
{
	return FindCorrectedQuatAtInputKeyAlongSpline(Spline, Spline->GetInputKeyAtDistanceAlongSpline(DistanceAlongSpline), CoordinateSpace);
}

FQuat USplineGenBPLibrary::FindCorrectedQuatAtInputKeyAlongSpline(const USplineComponent* Spline, const float InputKey, const ESplineCoordinateSpace::Type CoordinateSpace)
{
	float InputKeyFloor = FMath::Floor(InputKey);
	float LocalKey = InputKey - InputKeyFloor;
	int SplinePoint = int(InputKeyFloor);
	float StartTanLen = Spline->GetLeaveTangentAtSplinePoint(SplinePoint, ESplineCoordinateSpace::Local).Length();
	float EndTanLen = Spline->GetArriveTangentAtSplinePoint(GetNextSplinePoint(Spline, SplinePoint), ESplineCoordinateSpace::Local).Length();
	float SegLen = GetSegmentLength(Spline, SplinePoint);
	LocalKey = GenRollCurve(LocalKey, 0.f, 1.f, StartTanLen, EndTanLen, SegLen);
	float NewKey = InputKeyFloor + LocalKey;
	FQuat StartQuat = Spline->GetQuaternionAtSplinePoint(SplinePoint, ESplineCoordinateSpace::Local);
	FQuat EndQuat = Spline->GetQuaternionAtSplinePoint(GetNextSplinePoint(Spline, SplinePoint), ESplineCoordinateSpace::Local);
	FQuat InterpedQuat = FQuat::Slerp(StartQuat, EndQuat, LocalKey);
	return InterpedQuat;
}

/*int USplineGenBPLibrary::IntWrapAround(int Value, int Min, int Max, bool IsAdding)
{
	
}*/