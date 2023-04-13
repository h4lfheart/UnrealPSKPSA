#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PskxFactory.generated.h"

UCLASS()
class UNREALPSKPSA_API UPskxFactory : public UFactory
{
	GENERATED_BODY()
public:
	UPskxFactory()
	{
		bEditorImport = true;
		bText = false;

		Formats.Add(FactoryExtension + ";" + FactoryDescription);

		SupportedClass = FactoryClass;
	}
	
	static UObject* Import(const FString& Filename, UObject* Parent, const FName Name, const EObjectFlags Flags, TMap<FString, FString>
						   MaterialNameToPathMap);

protected:
	UClass* FactoryClass = UStaticMesh::StaticClass();
	FString FactoryExtension = "pskx";
	FString FactoryDescription = "Unreal Static Mesh";

	virtual bool FactoryCanImport(const FString& Filename) override
	{
		const auto Extension = FPaths::GetExtension(Filename);
		return Extension.Equals(FactoryExtension);
	}
	
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn, bool& bOutOperationCanceled) override
	{
		return Import(Filename, InParent, FName(*InName.ToString().Replace(TEXT("_LOD0"), TEXT(""))), Flags, TMap<FString, FString>());
	}
};
