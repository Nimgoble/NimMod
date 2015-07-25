// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NimModBlueprintFunctionLibrary.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ESortDirection: uint8
{
	ASCENDING UMETA(DisplayName = "Ascending"),
	DESCENDING UMETA(DisplayName = "Descending")
};

UCLASS()
class NIMMOD_API UNimModBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (FriendlyName = "Order By"), Category = "Utilities|Array")
	static void OrderArrayBy(const TArray<AActor*>& TargetArray, const FString &FieldName, TArray<AActor*>& OrderedArray);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (DisplayName = "Sort Array", CompactNodeTitle = "SORTARRAY", ArrayParm = "TargetArray|ArrayProperty"), Category = "Utilities|Array")
	static void Array_Sort(const TArray<int32>& TargetArray, const UArrayProperty* ArrayProperty, const FString &FieldName, ESortDirection SortDirection);
	static void GenericArray_Sort(void* TargetArray, const UArrayProperty* ArrayProp, const FString &FieldName, ESortDirection SortDirection);

	/*UFUNCTION(BlueprintCallable, CustomThunk, meta = (DisplayName = "Sort Array By Function", CompactNodeTitle = "SORTARRAYBYFUNCTION", ArrayParm = "TargetArray|ArrayProperty"), Category = "Utilities|Array")
	static void Array_Sort_By_Function(const TArray<int32>& TargetArray, const UArrayProperty* ArrayProperty, const FString &FunctionName, ESortDirection SortDirection);
	static void GenericArray_Sort_By_Function(void* TargetArray, const UArrayProperty* ArrayProp, const FString &FunctionName, ESortDirection SortDirection);*/
	
	DECLARE_FUNCTION(execArray_Sort)
	{
		Stack.StepCompiledIn<UArrayProperty>(NULL);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;

		P_GET_OBJECT(UArrayProperty, ArrayProperty);
		PARAM_PASSED_BY_REF(FieldName, UStrProperty, FString);
		PARAM_PASSED_BY_VAL(SortDirection, UByteProperty, ESortDirection);
		P_FINISH;

		GenericArray_Sort(ArrayAddr, ArrayProperty, FieldName, SortDirection);
	}

	/*DECLARE_FUNCTION(execArray_Sort_By_Function)
	{
		Stack.StepCompiledIn<UArrayProperty>(NULL);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;

		P_GET_OBJECT(UArrayProperty, ArrayProperty);
		PARAM_PASSED_BY_REF(FunctionName, UStrProperty, FString);
		PARAM_PASSED_BY_VAL(SortDirection, UByteProperty, ESortDirection);
		P_FINISH;

		GenericArray_Sort_By_Function(ArrayAddr, ArrayProperty, FunctionName, SortDirection);
	}*/
};

struct FArraySortByFieldPredicate
{
	FArraySortByFieldPredicate(const FString &InFieldName, ESortDirection InSortDirection) 
		: FieldName(InFieldName), SortDirection(InSortDirection)
	{
	}

	bool operator ()(const AActor& A, const AActor& B) const
	{
		/*if (A == nullptr || B == nullptr)
		return false;*/
		UClass *ourClass = A.GetClass();
		if (ourClass != B.GetClass())
			return false;

		UProperty *targetProperty = FindField<UProperty>(ourClass, *FieldName);
		if (targetProperty == nullptr)
			return false;

		const void *Aa = (SortDirection == ESortDirection::ASCENDING) ? &A : &B;
		const void *Bb = (SortDirection == ESortDirection::ASCENDING) ? &B : &A;

		if (targetProperty->IsA<UByteProperty>())
		{
			return Cast<UByteProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UByteProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UIntProperty>())
		{
			return
				Cast<UIntProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UIntProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UUInt32Property>())
		{
			return
				Cast<UUInt32Property>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UUInt32Property>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UFloatProperty>())
		{
			return
				Cast<UFloatProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UFloatProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UDoubleProperty>())
		{
			return
				Cast<UDoubleProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UDoubleProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UStrProperty>())
		{
			return
				Cast<UStrProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UStrProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UNameProperty>())
		{
			return
				Cast<UNameProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
				Cast<UNameProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
		}
		else if (targetProperty->IsA<UTextProperty>())
		{
			return
				Cast<UTextProperty>(targetProperty)->GetPropertyValue_InContainer(Aa).ToString() <
				Cast<UTextProperty>(targetProperty)->GetPropertyValue_InContainer(Bb).ToString();
		}
		// fall back, just let diff type win:
		else
			return false;
	}

	FString FieldName;
	ESortDirection SortDirection;
};

//struct FArraySortByFunctionPredicate
//{
//	FArraySortByFunctionPredicate(const FString &InFunctionName, ESortDirection InSortDirection)
//		: FunctionName(*InFunctionName), SortDirection(InSortDirection)
//	{
//	}
//
//	bool operator ()(const AActor& A, const AActor& B) const
//	{
//		/*if (A == nullptr || B == nullptr)
//		return false;*/
//		UClass *ourClass = A.GetClass();
//		if (ourClass != B.GetClass())
//			return false;
//
//		//UProperty *targetProperty = FindField<UProperty>(ourClass, *FieldName);
//		UFunction *targetAFunction = A.FindFunction(FunctionName);
//		UFunction *targetBFunction = B.FindFunction(FunctionName);
//		if (targetAFunction == nullptr || targetBFunction == nullptr)
//			return false;
//
//		UProperty *targetAProperty = targetAFunction->GetReturnProperty();
//		//FFrame currentStack = FFrame(*A, targetAFunction, )
//		A.CallFunction()
//		/*FStack::
//		A.CallFunction()*/
//
//		const void *Aa = (SortDirection == ESortDirection::ASCENDING) ? &A : &B;
//		const void *Bb = (SortDirection == ESortDirection::ASCENDING) ? &B : &A;
//
//		//if (targetProperty->IsA<UByteProperty>())
//		//{
//		//	return Cast<UByteProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UByteProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UIntProperty>())
//		//{
//		//	return
//		//		Cast<UIntProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UIntProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UUInt32Property>())
//		//{
//		//	return
//		//		Cast<UUInt32Property>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UUInt32Property>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UFloatProperty>())
//		//{
//		//	return
//		//		Cast<UFloatProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UFloatProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UDoubleProperty>())
//		//{
//		//	return
//		//		Cast<UDoubleProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UDoubleProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UStrProperty>())
//		//{
//		//	return
//		//		Cast<UStrProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UStrProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UNameProperty>())
//		//{
//		//	return
//		//		Cast<UNameProperty>(targetProperty)->GetPropertyValue_InContainer(Aa) <
//		//		Cast<UNameProperty>(targetProperty)->GetPropertyValue_InContainer(Bb);
//		//}
//		//else if (targetProperty->IsA<UTextProperty>())
//		//{
//		//	return
//		//		Cast<UTextProperty>(targetProperty)->GetPropertyValue_InContainer(Aa).ToString() <
//		//		Cast<UTextProperty>(targetProperty)->GetPropertyValue_InContainer(Bb).ToString();
//		//}
//		//// fall back, just let diff type win:
//		//else
//		//	return false;
//
//		return false;
//	}
//
//	FName FunctionName;
//	ESortDirection SortDirection;
//};

struct FNimModCustomThunkTemplates
{
	template<typename T>
	static void Array_Sort(TArray<T>& TargetArray, const UArrayProperty* ArrayProperty, const FString &FieldName, ESortDirection SortDirection)
	{
		UNimModBlueprintFunctionLibrary::GenericArray_Sort(&TargetArray, ArrayProperty, FieldName, SortDirection);
	}

	/*template<typename T>
	static void Array_Sort_By_Function(TArray<T>& TargetArray, const UArrayProperty* ArrayProperty, const FString &FunctionName, ESortDirection SortDirection)
	{
		UNimModBlueprintFunctionLibrary::GenericArray_Sort_By_Function(&TargetArray, ArrayProperty, FunctionName, SortDirection);
	}*/
};
