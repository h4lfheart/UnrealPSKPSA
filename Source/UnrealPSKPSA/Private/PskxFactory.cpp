#include "PskxFactory.h"

#include "PskPsaUtils.h"
#include "PskReader.h"
#include "RawMesh.h"
#include "Materials/MaterialInstanceConstant.h"

UObject* UPskxFactory::Import(const FString& Filename, UObject* Parent, const FName Name, const EObjectFlags Flags, TMap<FString, FString> MaterialNameToPathMap)
{
	auto Data = FPskReader(Filename);
	if (!Data.bIsValid) return nullptr;
	
	TArray<FColor> VertexColorsByPoint;
	VertexColorsByPoint.Init(FColor::Black, Data.VertexColors.Num());
	if (Data.bHasVertexColors)
	{
		for (auto i = 0; i < Data.Wedges.Num(); i++)
		{
			auto FixedColor = Data.VertexColors[i];
			Swap(FixedColor.R, FixedColor.B);
			VertexColorsByPoint[Data.Wedges[i].PointIndex] = FixedColor;
		}
	}

	// TODO STATIC MESH DESCRIPTION
	auto RawMesh = FRawMesh();
	for (auto Vertex : Data.Vertices)
	{
		auto FixedVertex = Vertex;
		FixedVertex.Y = -FixedVertex.Y; // MIRROR_MESH
		RawMesh.VertexPositions.Add(FixedVertex);
	}

	auto WindingOrder = {2, 1, 0};
	for (const auto PskFace : Data.Faces)
	{
		RawMesh.FaceMaterialIndices.Add(PskFace.MatIndex);
		RawMesh.FaceSmoothingMasks.Add(1);

		for (auto VertexIndex : WindingOrder)
		{
			const auto WedgeIndex = PskFace.WedgeIndex[VertexIndex];
			const auto PskWedge = Data.Wedges[WedgeIndex];

			RawMesh.WedgeIndices.Add(PskWedge.PointIndex);
			RawMesh.WedgeColors.Add(Data.bHasVertexColors ? VertexColorsByPoint[PskWedge.PointIndex] : FColor::Black);
			RawMesh.WedgeTexCoords[0].Add(FVector2f(PskWedge.U, PskWedge.V));
			for (auto UVIndex = 0; UVIndex < Data.ExtraUVs.Num(); UVIndex++)
			{
				auto UV =  Data.ExtraUVs[UVIndex][PskFace.WedgeIndex[VertexIndex]];
				RawMesh.WedgeTexCoords[UVIndex+1].Add(UV);
			}
			
			RawMesh.WedgeTangentZ.Add(Data.bHasVertexNormals ? Data.Normals[PskWedge.PointIndex] * FVector3f(1, -1, 1) : FVector3f::ZeroVector);
			RawMesh.WedgeTangentY.Add(FVector3f::ZeroVector);
			RawMesh.WedgeTangentX.Add(FVector3f::ZeroVector);
		}
	}
	
	const auto StaticMesh = FPskPsaUtils::LocalCreate<UStaticMesh>(UStaticMesh::StaticClass(), Parent, Name.ToString(), Flags);

	for (auto i = 0; i < Data.Materials.Num(); i++)
	{
		auto PskMaterial = Data.Materials[i];
		
		UObject* MatParent;
		auto FoundMaterialPath = MaterialNameToPathMap.Find(PskMaterial.MaterialName);
		if (FoundMaterialPath != nullptr)
		{
			MatParent = CreatePackage(**FoundMaterialPath);
		}
		else
		{
			MatParent = Parent;
		}
		
		auto MaterialAdd = FPskPsaUtils::LocalFindOrCreate<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), MatParent, PskMaterial.MaterialName, Flags);

		StaticMesh->GetStaticMaterials().Add(FStaticMaterial(MaterialAdd));
		StaticMesh->GetSectionInfoMap().Set(0, i, FMeshSectionInfo(i));
	}
	
	auto& SourceModel = StaticMesh->AddSourceModel();
	SourceModel.BuildSettings.bGenerateLightmapUVs = false;
	SourceModel.BuildSettings.bBuildReversedIndexBuffer = false;
	SourceModel.BuildSettings.bRemoveDegenerates = true;
	SourceModel.BuildSettings.bRecomputeNormals = !Data.bHasVertexNormals;
	SourceModel.BuildSettings.bRecomputeTangents = true;
	SourceModel.BuildSettings.bUseMikkTSpace = true;
	SourceModel.SaveRawMesh(RawMesh);

	StaticMesh->Build();
	StaticMesh->PostEditChange();
	FAssetRegistryModule::AssetCreated(StaticMesh);
	StaticMesh->MarkPackageDirty();
	
	FGlobalComponentReregisterContext RecreateComponents;

	return StaticMesh;
}
