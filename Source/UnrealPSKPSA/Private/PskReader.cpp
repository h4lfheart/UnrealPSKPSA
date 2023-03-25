#include "PskReader.h"

#include <fstream>

#include "UnrealPSKPSA.h"

FPskReader::FPskReader(const FString& Filepath)
{
	std::ifstream Ar;
	Ar.open(ToCStr(Filepath), std::ios::binary);

	const FPskHeader MainHeader(Ar);
	if (!MainHeader.ChunkName.Equals("ACTRHEAD"))
	{
		return;
	}

	while (!Ar.eof())
	{
		FPskHeader Header(Ar);
		const auto Name = Header.ChunkName;
		const auto Count = Header.Count;
		const auto Size = Header.Count;

		UE_LOG(LogUnrealPSKPSA, Log, TEXT("%s: %d"), *Name, Count);
		
		if (Name.Equals("PNTS0000"))
		{
			Vertices.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Vertices[i]), sizeof(FVector3f));
			}
		}
		else if (Name.Equals("VTXW0000"))
		{
			Wedges.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Wedges[i]), sizeof(VVertex));
				if (Count <= 65536)
				{
					Wedges[i].PointIndex &= 0xFFFF;
				}
			}
		}
		else if (Name.Equals("FACE0000"))
		{
			Faces.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				for (auto j = 0; j < 3; j++)
				{
					Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex[j]), sizeof(short));
					Faces[i].WedgeIndex[j] &= 0xFFFF;
				}
	                
				Ar.read(&Faces[i].MatIndex, sizeof(char));
				Ar.read(&Faces[i].AuxMatIndex, sizeof(char));
				Ar.read(reinterpret_cast<char*>(&Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (Name.Equals("FACE3200"))
		{
			Faces.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex), sizeof Faces[i].WedgeIndex);
				Ar.read(&Faces[i].MatIndex, sizeof(char));
                Ar.read(&Faces[i].AuxMatIndex, sizeof(char));
                Ar.read(reinterpret_cast<char*>(&Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (Name.Equals("MATT0000"))
		{
			Materials.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Materials[i]), sizeof(VMaterial));
			}
		}
		else if (Name.Equals("VTXNORMS"))
		{
			Normals.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Normals[i]), sizeof(FVector3f));
			}
		}
		else if (Name.Equals("VERTEXCOLOR"))
		{
			VertexColors.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&VertexColors[i]), sizeof(FColor));
			}
		}
		else if (Name.Equals("EXTRAUVS"))
		{
			TArray<FVector2f> UVData;
			UVData.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&UVData[i]), sizeof(FVector2f));
			}

			ExtraUVs.Add(UVData);
		}
		else if (Name.Equals("REFSKELT") || Name.Equals("REFSKEL0"))
		{
			Bones.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Bones[i].Name), sizeof(Bones[i].Name));
				Ar.read(reinterpret_cast<char*>(&Bones[i].Flags), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].NumChildren), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].ParentIndex), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Orientation), sizeof(FQuat4f));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Position), sizeof(FVector3f));
				
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Length), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.XSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.YSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.ZSize), sizeof(float));
	            	
			}
		}
		else if (Name.Equals("RAWWEIGHTS") || Name.Equals("RAWW0000"))
		{
			Influences.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Influences[i]), sizeof(VRawBoneInfluence));
			}
		}
		else if (Name.Equals("MRPHINFO"))
		{
			MorphInfos.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&MorphInfos[i]), sizeof(VMorphInfo));
			}
		}
		else if (Name.Equals("MRPHDATA"))
		{
			MorphDatas.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&MorphDatas[i]), sizeof(VMorphData));
			}
		}
		else
		{
			Ar.ignore(Size*Count); 
		}
	}

	Ar.close();

	bIsValid = true;
	bHasVertexNormals = Normals.Num() > 0;
	bHasVertexColors = VertexColors.Num() > 0;
	bHasMorphData = MorphInfos.Num() > 0 && MorphDatas.Num() > 0;
}

