// Fill out your copyright notice in the Description page of Project Settings.

#include "SGMeshSplineComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
USGMeshSplineComponent::USGMeshSplineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void USGMeshSplineComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USGMeshSplineComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	SetComponentTickEnabled(false);
	DeleteAll();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void USGMeshSplineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSelection(CurrentSelection, FSectionStyle(), true);
	// ...
}

void USGMeshSplineComponent::UpdateSection(int SplinePoint, FSectionStyle Style, bool bUpdateTransforms)
{
	SplinePoint = WrapSplinePointToRange(SplinePoint);
	if (SplinePoint > GetLastSplinePoint())
	{
		DeleteSection(SplinePoint, false);
		return;
	}
	if (SplinePoint < 0) return;

	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	float StartDistSection = GetDistanceAlongSplineAtSplinePoint(SplinePoint);
	float EndDistSection = GetDistanceAlongSplineAtSplinePoint(GetNextSplinePoint(SplinePoint));
	float SectionLength = GetSegmentLength(SplinePoint);
	int MeshCount = FMath::RoundToInt(SectionLength / TargetMeshLength);
	float InKeyStep = 1.f / float(MeshCount);
	float MeshLength = SectionLength / MeshCount;

	UE_LOG(LogTemp, Display, TEXT("section mesh length: %f"), MeshLength);

	if (AllMeshes.Num() - 1 < SplinePoint) AllMeshes.SetNum(SplinePoint + 1);
	if (AllMeshes[SplinePoint].Meshes.Num() < MeshCount) AllMeshes[SplinePoint].Meshes.SetNum(MeshCount);

	int CurrentMeshesNum = AllMeshes[SplinePoint].Meshes.Num();

	

	// Cycle through all meshes to create/update.
	// Need to convert this to distance along spline!
	for (int i = 0; i < MeshCount; i++)
	{
		USplineMeshComponent* CurrentMesh = AllMeshes[SplinePoint].Meshes[i];
		float StartDistance = StartDistSection + (MeshLength * i);
		float EndDistance = (i < MeshCount - 1) ? (StartDistance + MeshLength) : EndDistSection; // Fixes the gap between the last mesh and next section.
		bool bIsFinalMesh = SplinePoint >= GetLastSplinePoint() && i >= MeshCount - 1;
		if (bIsFinalMesh) EndDistance = LocalOffset.Length() * 0.5f;
		//float LastDistance = IsClosedLoop() ? FMath::Wrap((StartDistance - MeshLength), 0.f, GetSplineLength()) : (StartDistance - MeshLength);
		//float NextDistance = IsClosedLoop() ? FMath::Wrap((EndDistance + MeshLength), 0.f, GetSplineLength()) : (EndDistance + MeshLength);

		// Spawn spline mesh if not exist.
		if (!CurrentMesh)
		{
			FName NewComponentName = MakeUniqueObjectName(GetOwner(), USplineMeshComponent::StaticClass(), FName("TrackSplineMeshComponent"));
			CurrentMesh = NewObject<USplineMeshComponent>(GetOwner(), USplineMeshComponent::StaticClass(), NewComponentName);
			CurrentMesh->RegisterComponent();
			CurrentMesh->SetMobility(EComponentMobility::Movable);
			CurrentMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
		}

		// Cosmetic updates.
		Style = GetFallbackStyle(Style, SplinePoint);
		if (Style.Mesh) CurrentMesh->SetStaticMesh(Style.Mesh);
		for (int j = 0; j < Style.Materials.Num(); j++) if (Style.Materials[j]) CurrentMesh->SetMaterial(j, Style.Materials[j]);

		// Transform updates.
		FVector StartLoc = GetLocationAtDistanceAlongSpline(StartDistance, LS);
		FVector EndLoc = GetLocationAtDistanceAlongSpline(EndDistance, LS);
		FVector StartTan = GetTangentAtDistanceAlongSpline(StartDistance, LS) * InKeyStep;
		FVector EndTan = GetTangentAtDistanceAlongSpline(EndDistance, LS) * InKeyStep;
		FVector OverrideTangentNext = FVector::ZeroVector;
		if (bEnableLocalOffset)
		{
			if (bEnableSmoothTangentsForLocalOffset)
			{
				StartLoc = GetLocalOffsetLocationAtDistanceAlongSplineFromOffsetSpline(StartDistance, LS);
				EndLoc = GetLocalOffsetLocationAtDistanceAlongSplineFromOffsetSpline(EndDistance, LS);
				StartTan = GetLocalOffsetTangentAtDistanceAlongSplineFromOffsetSpline(StartDistance, LS);
				EndTan = GetLocalOffsetTangentAtDistanceAlongSplineFromOffsetSpline(EndDistance, LS);
				OverrideTangentNext = EndTan;
			}
			else
			{
				StartLoc = GetLocalOffsetLocationAtDistanceAlongSpline(StartDistance, LocalOffset, LS);
				EndLoc = GetLocalOffsetLocationAtDistanceAlongSpline(EndDistance, LocalOffset, LS);
			}
		}
		CurrentMesh->SetStartAndEnd(StartLoc, StartTan, EndLoc, EndTan, false);

		CurrentMesh->SetStartScale(MapScaleTo2D(GetScaleAtDistanceAlongSpline(StartDistance)));
		CurrentMesh->SetEndScale(MapScaleTo2D(GetScaleAtDistanceAlongSpline(EndDistance)));

		CurrentMesh->SetSplineUpDir(GetCorrectUpVectorAtDistanceAlongSpline(StartDistance, LS), true);

		float EndRoll = FindDeltaRollAtDistanceAlongSpline(StartDistance, EndDistance, OverrideTangentNext);
		CurrentMesh->SetStartRoll(0.f, false);
		CurrentMesh->SetEndRoll(EndRoll, false);

		// Final updates.
		CurrentMesh->UpdateMesh();
		AllMeshes[SplinePoint].Meshes[i] = CurrentMesh;
	}

	// Check for unused meshes, despawn them and remove from array.
	if (MeshCount < CurrentMeshesNum)
	{
		for (int i = CurrentMeshesNum-1; i > MeshCount-1; i--)
		{
			AllMeshes[SplinePoint].Meshes[i]->DestroyComponent();
			AllMeshes[SplinePoint].Meshes.RemoveAt(i);
		}
	}
}

FVector2D USGMeshSplineComponent::MapScaleTo2D(FVector Scale)
{
	return FVector2D(Scale.Y, Scale.Z);
}

float USGMeshSplineComponent::FindDeltaRollAtDistanceAlongSpline(float StartDist, float EndDist, FVector OverrideTangentNext)
{
	ESplineCoordinateSpace::Type WS = ESplineCoordinateSpace::World;
	ESplineCoordinateSpace::Type LS = ESplineCoordinateSpace::Local;

	//FRotator Start = GetCorrectRotationAtDistanceAlongSpline(StartDist, LS);
	//FRotator End = GetCorrectRotationAtDistanceAlongSpline(EndDist, LS);
	//FQuat Start = GetCorrectQuaternionAtDistanceAlongSpline(StartDist, LS);
	//FQuat End = GetCorrectQuaternionAtDistanceAlongSpline(EndDist, LS);
	
	FVector TangentNext;// = GetTangentAtDistanceAlongSpline(EndDist, WS);
	if (OverrideTangentNext == FVector::ZeroVector) TangentNext = GetTangentAtDistanceAlongSpline(EndDist, WS);
	else TangentNext = OverrideTangentNext;
	FVector UpVectorCurrent = GetCorrectUpVectorAtDistanceAlongSpline(StartDist, WS);
	FVector UpVectorNext = GetCorrectUpVectorAtDistanceAlongSpline(EndDist, WS);
	FVector CrossProductCurrent = FVector::CrossProduct(TangentNext.GetSafeNormal(), UpVectorCurrent);
	FVector CrossProductNext = FVector::CrossProduct(TangentNext.GetSafeNormal(), UpVectorNext);
	FVector CrossProductBoth = FVector::CrossProduct(CrossProductCurrent, CrossProductNext).GetSafeNormal();
	float DotProductA = FVector::DotProduct(CrossProductCurrent.GetSafeNormal(), CrossProductNext.GetSafeNormal());
	float DotProductB = FVector::DotProduct(CrossProductBoth.GetSafeNormal(), TangentNext);
	float Result = UKismetMathLibrary::SignOfFloat(DotProductB) * -1 * FGenericPlatformMath::Acos(DotProductA);
	//float Result = FMath::RadiansToDegrees(UKismetMathLibrary::SignOfFloat(DotProductB) * -1 * FGenericPlatformMath::Acos(DotProductA));

	//float IncorrectResult = (End.W - Start.W);
	//float IncorrectResult = FMath::UnwindDegrees(FMath::UnwindDegrees(End.W) - FMath::UnwindDegrees(Start.W));
	//float IncorrectResult = (End.Roll - Start.Roll);
	//UE_LOG(LogTemp, Display, TEXT("Correct Result: %f - Incorrect Result: %f"), Result, IncorrectResult);
	
	return Result;
}

void USGMeshSplineComponent::UpdateAll(TArray<FSectionStyle> Styles, bool bUpdateTransforms)
{
	UpdateSGSplines();
	for (int i = 0; i < GetNumberOfSplineSegments(); i++)
	{
		FSectionStyle UseStyle = DefaultStyle;
		if (Styles.IsValidIndex(i)) UseStyle = Styles[i];
		UpdateSection(i, UseStyle, bUpdateTransforms);
	}
}

void USGMeshSplineComponent::UpdateSelection(TArray<int> Selection, FSectionStyle Style, bool bUpdateTransforms)
{
	UpdateSGSplines();
	Selection.Sort();
	TArray<int> AffectedSegments;
	for (int i = 0; i < Selection.Num(); i++)
	{
		int PreviousPointInSelection = Selection[GetNextPreviousInRange(i, 0, Selection.Num() - 1, false)];
		int PointBeforeSelectionOnSpline = GetPreviousSplinePoint(Selection[i]);
		bool bIsPreviousPointInSelection = (PreviousPointInSelection == PointBeforeSelectionOnSpline);
		if (!bIsPreviousPointInSelection) AffectedSegments.Emplace(PointBeforeSelectionOnSpline);
	}
	AffectedSegments.Append(Selection);
	AffectedSegments.Sort();
	for (int SplinePoint : AffectedSegments) 
	{
		UpdateSection(SplinePoint, GetFallbackStyle(Style, SplinePoint), bUpdateTransforms);
	}
}

TArray<int> USGMeshSplineComponent::GetCurrentSelection()
{
	return CurrentSelection;
}

void USGMeshSplineComponent::SetCurrentSelection(TArray<int> NewSelection)
{
	CurrentSelection = NewSelection;
}

void USGMeshSplineComponent::AddToSelection(int Point)
{
	CurrentSelection.Emplace(Point);
}

void USGMeshSplineComponent::RemoveFromSelection(int Point)
{
	CurrentSelection.Remove(Point);
}

float USGMeshSplineComponent::GetTargetMeshLength()
{
	return TargetMeshLength;
}

void USGMeshSplineComponent::SetTargetMeshLength(float NewTargetMeshLength)
{
	TargetMeshLength = NewTargetMeshLength;
}

void USGMeshSplineComponent::SetStyleOnSelected(FSectionStyle Style)
{
	UpdateSelection(CurrentSelection, Style, false);
}

void USGMeshSplineComponent::StartSelectionEditOperation()
{
	SetComponentTickEnabled(true);
}

void USGMeshSplineComponent::EndSelectionEditOperation()
{
	SetComponentTickEnabled(false);
}

void USGMeshSplineComponent::DeleteSection(int Section, bool bUpdate)
{
	if (AllMeshes.IsValidIndex(Section))
	{
		for (auto Mesh : AllMeshes[Section].Meshes)
		{
			if (Mesh) Mesh->DestroyComponent();
		}
		AllMeshes.RemoveAt(Section);
	}
	if (bUpdate) UpdateSection(GetPreviousSplinePoint(Section));
}

void USGMeshSplineComponent::DeleteUnusedSections()
{
	for (int i = AllMeshes.Num() - 1; i > GetLastSegment(); i--)
	{
		DeleteSection(i, false);
	}
}

void USGMeshSplineComponent::DeleteAll()
{
	for (auto Meshes : AllMeshes)
	{
		for (auto Mesh : Meshes.Meshes)
		{
			if (Mesh) Mesh->DestroyComponent();
		}
	}
	AllMeshes.Empty();
}

void USGMeshSplineComponent::AddNewSection(FSectionStyle Style)
{
	UpdateSection(GetLastSegment(), Style, true);
}

FSectionStyle USGMeshSplineComponent::GetDefaultStyle()
{
	return DefaultStyle;
}

void USGMeshSplineComponent::SetDefaultStyle(FSectionStyle NewStyle)
{
	DefaultStyle = NewStyle;
}

FSectionStyle USGMeshSplineComponent::GetStyleFromSplineMesh(USplineMeshComponent* Mesh)
{
	if (!Mesh) return FSectionStyle();
	FSectionStyle Style = FSectionStyle(
		Mesh->GetStaticMesh(),
		Mesh->GetMaterials()
	);
	return Style;
}

FSectionStyle USGMeshSplineComponent::GetFallbackStyle(FSectionStyle Style, int SplinePoint)
{
	if (!Style.Mesh)
	{
		if (AllMeshes.IsValidIndex(SplinePoint))
		{
			if (AllMeshes[SplinePoint].Meshes.IsValidIndex(0)) {
				if (AllMeshes[SplinePoint].Meshes[0]) Style = GetStyleFromSplineMesh(AllMeshes[SplinePoint].Meshes[0]);
			}
		}
		if (!Style.Mesh) Style = GetDefaultStyle();
	}
	return Style;
}

TArray<FSplineMeshSection> USGMeshSplineComponent::GetAllMeshes()
{
	return AllMeshes;
}

void USGMeshSplineComponent::SetAllMeshes(TArray<FSplineMeshSection> NewMeshes)
{
	AllMeshes = NewMeshes;
}

void USGMeshSplineComponent::SplineSetClosedLoop(bool bInClosedLoop, bool bUpdateSpline)
{
	bool bPreviousIsClosedLoop = IsClosedLoop();
	SetClosedLoop(bInClosedLoop, bUpdateSpline);
	if (!bPreviousIsClosedLoop && bInClosedLoop)
	{
		UpdateSection(
			GetLastSegment(),
			GetFallbackStyle(
				FSectionStyle(),
				(GetPreviousSplinePoint(GetLastSegment(), true))
			),
			true
		);
	}
	else if (bPreviousIsClosedLoop && !bInClosedLoop)
	{
		DeleteSection(GetLastSegment() + 1, true);
	}
}
