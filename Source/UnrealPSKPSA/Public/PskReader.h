#pragma once
#include <fstream>

#include "ActorXModels.h"

class FPskHeader
{
public:
	FString ChunkName;
	int Size;
	int Count;
	
	FPskHeader(std::ifstream& Ar)
	{
		VChunkHeader Header;
		Ar.read(reinterpret_cast<char*>(&Header), sizeof(VChunkHeader));

		ChunkName = FString(UTF8_TO_TCHAR(Header.ChunkID));
		Size = Header.DataSize;
		Count = Header.DataCount;
	}
};

class UNREALPSKPSA_API FPskReader
{
public:
	FPskReader(const FString& Filepath);
	
	bool bIsValid = false;
	bool bHasVertexNormals = false;
	bool bHasVertexColors = false;
	bool bHasMorphData = false;
	
	TArray<FVector3f> Vertices;
	TArray<VVertex> Wedges;
	TArray<VTriangle> Faces;
	TArray<VMaterial> Materials;
	TArray<FVector3f> Normals;
	TArray<FColor> VertexColors;
	TArray<TArray<FVector2f>> ExtraUVs;
	TArray<VMorphInfo> MorphInfos;
	TArray<VMorphData> MorphDatas;

	TArray<VNamedBoneBinary> Bones;
	TArray<VRawBoneInfluence> Influences;
};