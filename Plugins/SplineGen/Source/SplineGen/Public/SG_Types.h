#pragma once

//#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
//#include "UObject/NoExportTypes.h"

#include "SG_Types.generated.h"

UENUM(BlueprintType)
enum class EInterpInOutType : uint8
{
	Linear                  UMETA(DisplayName = "Linear", Tooltip = ""),
	Circular                UMETA(DisplayName = "Circular", Tooltip = ""),
	Ease                    UMETA(DisplayName = "Ease", Tooltip = ""),
	Expo                    UMETA(DisplayName = "Expo", Tooltip = ""),
	Sine                    UMETA(DisplayName = "Sine", Tooltip = ""),
	AutoEase                UMETA(DisplayName = "AutoEase", Tooltip = "")
};

UENUM(BlueprintType)
enum class EInterpInOutSelection : uint8
{
	None                    UMETA(DisplayName = "None", Tooltip = ""),
	EaseIn                  UMETA(DisplayName = "EaseIn", Tooltip = ""),
	EaseOut                 UMETA(DisplayName = "EaseOut", Tooltip = ""),
	EaseInOut               UMETA(DisplayName = "EaseInOut", Tooltip = "")
};

USTRUCT(BlueprintType)
struct FRollConfig
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInterpInOutType EaseType = EInterpInOutType::AutoEase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInterpInOutSelection InOut = EInterpInOutSelection::EaseInOut;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EaseExp = 1.0f;

	FRollConfig()
		: EaseType(EInterpInOutType::Ease),
		InOut(EInterpInOutSelection::EaseInOut),
		EaseExp(1.0f)
	{}

	FRollConfig(EInterpInOutType EaseType, EInterpInOutSelection InOut, float EaseExp)
		: EaseType(EaseType),
		InOut(InOut),
		EaseExp(EaseExp)
	{}
};

USTRUCT(BlueprintType)
struct FSplineSegment
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Segment = -1;
};

USTRUCT(BlueprintType)
struct FSplinePointSetting
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESplineCoordinateSpace::Type> LocationCoordSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESplineCoordinateSpace::Type> UpVectorCoordSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector UpVector = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESplineCoordinateSpace::Type> TangentCoordSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Tangent = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	FSplinePointSetting()
		: LocationCoordSpace(ESplineCoordinateSpace::World),
		Location(FVector::ZeroVector),
		UpVectorCoordSpace(ESplineCoordinateSpace::World),
		UpVector(FVector::UpVector),
		TangentCoordSpace(ESplineCoordinateSpace::Local),
		Tangent(FVector::ForwardVector),
		Scale(FVector::OneVector)
	{}
	
	FSplinePointSetting(ESplineCoordinateSpace::Type LocationCoordSpace, FVector Location, ESplineCoordinateSpace::Type UpVectorCoordSpace, FVector UpVector, ESplineCoordinateSpace::Type TangentCoordSpace, FVector Tangent, FVector Scale)
		: LocationCoordSpace(ESplineCoordinateSpace::World),
		Location(FVector::ZeroVector),
		UpVectorCoordSpace(ESplineCoordinateSpace::World),
		UpVector(FVector::UpVector),
		TangentCoordSpace(ESplineCoordinateSpace::Local),
		Tangent(FVector::ForwardVector),
		Scale(FVector::OneVector)
	{}
};

USTRUCT(BlueprintType)
struct FMeshSplineDivisions
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UserSegmentLength = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SegmentsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResultSegmentLength = 0.0f;

	FMeshSplineDivisions()
		: UserSegmentLength(20.0f), SegmentsCount(0), ResultSegmentLength(0.0f)
	{}

	//FMeshSplineDivisions(float UserSegmentLength, int SegmentsCount, float ResultSegmentLength)
	//	: UserSegmentLength(UserSegmentLength), SegmentsCount(SegmentsCount), ResultSegmentLength(ResultSegmentLength)
	//{}
};

/*
USTRUCT(BlueprintType)
struct FControlPointMap
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ControlPointA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ControlPointB;

	FControlPointMap()
		: ControlPointA(0), ControlPointB(0)
	{}

	FControlPointMap(int Val)
		: ControlPointA(Val), ControlPointB(Val)
	{}

	FControlPointMap(int A, int B)
		: ControlPointA(A), ControlPointB(B)
	{}
};
*/