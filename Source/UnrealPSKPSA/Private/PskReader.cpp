#include "PskReader.h"

#include <fstream>

FPskData FPskReader::Read(FString Filepath)
{
	FPskData Data;
	
	std::ifstream Ar(ToCStr(Filepath), std::ios::binary);

	const FPskHeader MainHeader(Ar);
	if (!MainHeader.ChunkName.Equals("ACTRHEAD"))
	{
		return Data;
	}

	while (!Ar.eof())
	{
		FPskHeader Header(Ar);
		const auto Name = Header.ChunkName;
		const auto Count = Header.Count;
		const auto Size = Header.Count;
		
		if (Name.Equals("PNTS0000"))
		{
			Data.Vertices.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Vertices[i]), sizeof(FVector3f));
			}
		}
		else if (Name.Equals("VTXW0000"))
		{
			Data.Wedges.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Wedges[i]), sizeof(VVertex));
				if (Count <= 65536)
				{
					Data.Wedges[i].PointIndex &= 0xFFFF;
				}
			}
		}
		else if (Name.Equals("FACE0000"))
		{
			Data.Faces.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				for (auto j = 0; j < 3; j++)
				{
					Ar.read(reinterpret_cast<char*>(&Data.Faces[i].WedgeIndex[j]), sizeof(short));
					Data.Faces[i].WedgeIndex[j] &= 0xFFFF;
				}
	                
				Ar.read(&Data.Faces[i].MatIndex, sizeof(char));
				Ar.read(&Data.Faces[i].AuxMatIndex, sizeof(char));
				Ar.read(reinterpret_cast<char*>(&Data.Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (Name.Equals("FACE3200"))
		{
			Data.Faces.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Faces[i].WedgeIndex), sizeof Data.Faces[i].WedgeIndex);
				Ar.read(&Data.Faces[i].MatIndex, sizeof(char));
                Ar.read(&Data.Faces[i].AuxMatIndex, sizeof(char));
                Ar.read(reinterpret_cast<char*>(&Data.Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (Name.Equals("MATT0000"))
		{
			Data.Materials.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Materials[i]), sizeof(VMaterial));
			}
		}
		else if (Name.Equals("VTXNORMS"))
		{
			Data.Normals.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Normals[i]), sizeof(FVector3f));
			}
		}
		else if (Name.Equals("VERTEXCOLOR"))
		{
			Data.VertexColors.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.VertexColors[i]), sizeof(FColor));
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

			Data.ExtraUVs.Add(UVData);
		}
		else if (Name.Equals("REFSKELT") || Name.Equals("REFSKEL0"))
		{
			Data.Bones.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].Name), sizeof(Data.Bones[i].Name));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].Flags), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].NumChildren), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].ParentIndex), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.Orientation), sizeof(FQuat4f));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.Position), sizeof(FVector3f));
				
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.Length), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.XSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.YSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Data.Bones[i].BonePos.ZSize), sizeof(float));
	            	
			}
		}
		else if (Name.Equals("RAWWEIGHTS") || Name.Equals("RAWW0000"))
		{
			Data.Influences.SetNum(Count);
			for (auto i = 0; i < Count; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Data.Influences[i]), sizeof(VRawBoneInfluence));
			}
		}
		else
		{
			Ar.ignore(Size*Count); 
		}
	}

	Ar.close();

	Data.bIsValid = true;
	return Data;
	
}

