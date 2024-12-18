// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGSplineComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineMeshComponent.h"
#include "SGMeshSplineComponent.generated.h"

USTRUCT(BlueprintType)
struct FSplineMeshSection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USplineMeshComponent*> Meshes;
};

USTRUCT(BlueprintType)
struct FSectionStyle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;

	FSectionStyle()
		: Mesh(nullptr), Materials({})
	{}

	FSectionStyle(UStaticMesh* Mesh, TArray<UMaterialInterface*> Materials)
		: Mesh(Mesh), Materials(Materials)
	{}
};

USTRUCT(BlueprintType)
struct FSectionDivisions
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UserSegmentLength = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SegmentsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResultSegmentLength = 0.0f;

	FSectionDivisions()
		: UserSegmentLength(20.0f), SegmentsCount(0), ResultSegmentLength(0.0f)
	{}

	FSectionDivisions(float UserSegmentLength, int SegmentsCount, float ResultSegmentLength)
		: UserSegmentLength(UserSegmentLength), SegmentsCount(SegmentsCount), ResultSegmentLength(ResultSegmentLength)
	{}
};

/**
 * 
 */
UCLASS(Meta = (BlueprintSpawnableComponent))
class SPLINEGEN_API USGMeshSplineComponent : public USGSplineComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USGMeshSplineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	TArray<int> CurrentSelection = {};

	TArray<FSplineMeshSection> AllMeshes;
	
	UPROPERTY(EditAnywhere)
	FSectionStyle DefaultStyle = FSectionStyle();

protected:
	//TArray<TArray<USplineMeshComponent*>> AllMeshesNew; // needs more helper/setter/getter functions, probably not worth the trouble to expose to BP.

public:
	//UPROPERTY(BlueprintReadWrite, EditAnywhere)

	FVector test = UKismetMathLibrary::GetUpVector(FRotator::ZeroRotator);
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void UpdateSection(int SplinePoint, FSectionStyle Style = FSectionStyle(), bool bUpdateTransforms = true);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	FVector2D MapScaleTo2D(FVector Scale);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	float FindDeltaRollAtDistanceAlongSpline(float StartDist, float EndDist, FVector OverrideTangentNext = FVector::ZeroVector);

	// After loading a park/coaster.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void UpdateAll(TArray<FSectionStyle> Styles, bool bUpdateTransforms = true);

	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void UpdateSelection(TArray<int> Selection, FSectionStyle Style = FSectionStyle(), bool bUpdateTransforms = true);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	TArray<int> GetCurrentSelection();

	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SetCurrentSelection(TArray<int> NewSelection);
	
	// User selects a control point to edit.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void AddToSelection(int Point);

	// User deselects a control point to edit.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void RemoveFromSelection(int Point);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	float GetTargetMeshLength();

	// Modify this as needed. Might break things if used during runtime after calling UpdateAll.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SetTargetMeshLength(float NewTargetMeshLength = 100.f);

	// User applies a new style (track mesh, materials, etc.) on their selection.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SetStyleOnSelected(FSectionStyle Style);

	// User holds mouse button down to perform an edit. 
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void StartSelectionEditOperation();

	// User releases mouse button to finish performing an edit.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void EndSelectionEditOperation();

	// User deletes a track segment.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void DeleteSection(int Section, bool bUpdate = false);

	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void DeleteUnusedSections();

	// Automatically called when deleting this component or its actor. Can also be called from a reset/purge function for the track spline.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void DeleteAll();

	// User adds a track segment.
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void AddNewSection(FSectionStyle Style);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	FSectionStyle GetDefaultStyle();

	// Set this in BP actor!
	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SetDefaultStyle(FSectionStyle NewStyle);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	FSectionStyle GetStyleFromSplineMesh(USplineMeshComponent* Mesh);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	FSectionStyle GetFallbackStyle(FSectionStyle CheckStyle, int SplinePoint);

	UFUNCTION(BlueprintPure, meta = (Keywords = ""), Category = "")
	TArray<FSplineMeshSection> GetAllMeshes();

	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SetAllMeshes(TArray<FSplineMeshSection> NewMeshes);

	UFUNCTION(BlueprintCallable, meta = (Keywords = ""), Category = "")
	void SplineSetClosedLoop(bool bInClosedLoop, bool bUpdateSpline = true);
};