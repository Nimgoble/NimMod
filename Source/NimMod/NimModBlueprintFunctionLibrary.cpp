// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModBlueprintFunctionLibrary.h"

void UNimModBlueprintFunctionLibrary::OrderArrayBy(const TArray<AActor*>& TargetArray, const FString &FieldName, TArray<AActor*>& OrderedArray)
{
	OrderedArray.Append(TargetArray);
	OrderedArray.Sort(FArraySortByFieldPredicate(FieldName, ESortDirection::ASCENDING));
}

void UNimModBlueprintFunctionLibrary::Array_Sort(const TArray<int32>& TargetArray, const UArrayProperty* ArrayProp, const FString &FieldName, ESortDirection SortDirection)
{
	// We should never hit these!  They're stubs to avoid NoExport on the class.  Call the Generic* equivalent instead
	check(0);
}

void UNimModBlueprintFunctionLibrary::GenericArray_Sort(void* TargetArray, const UArrayProperty* ArrayProp, const FString &FieldName, ESortDirection SortDirection)
{
	if (TargetArray)
	{
		TArray<AActor *> *actorArray = (TArray<AActor *> *)TargetArray;
		if (actorArray != nullptr)
		{
			actorArray->Sort(FArraySortByFieldPredicate(FieldName, SortDirection));
		}
	}
}

//void UNimModBlueprintFunctionLibrary::Array_Sort_By_Function(const TArray<int32>& TargetArray, const UArrayProperty* ArrayProp, const FString &FunctionName, ESortDirection SortDirection)
//{
//	// We should never hit these!  They're stubs to avoid NoExport on the class.  Call the Generic* equivalent instead
//	check(0);
//}
//
//void UNimModBlueprintFunctionLibrary::GenericArray_Sort_By_Function(void* TargetArray, const UArrayProperty* ArrayProp, const FString &FunctionName, ESortDirection SortDirection)
//{
//	if (TargetArray)
//	{
//		TArray<AActor *> *actorArray = (TArray<AActor *> *)TargetArray;
//		if (actorArray != nullptr)
//		{
//			actorArray->Sort(FArraySortByFunctionPredicate(FunctionName, SortDirection));
//		}
//	}
//}

