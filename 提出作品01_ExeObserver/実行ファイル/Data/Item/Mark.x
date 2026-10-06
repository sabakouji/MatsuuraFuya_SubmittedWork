xof 0303txt 0032

template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template FrameTransformMatrix {
 <F6F23F41-7686-11cf-8F52-0040333594A3>
 Matrix4x4 frameMatrix;
}

template Frame {
 <3D82AB46-62DA-11cf-AB39-0020AF71E433>
 [...]
}


template XSkinMeshHeader {
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
 WORD nMaxSkinWeightsPerVertex;
 WORD nMaxSkinWeightsPerFace;
 WORD nBones;
}

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}

template SkinWeights {
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>
 STRING transformNodeName;
 DWORD nWeights;
 array DWORD vertexIndices[nWeights];
 array FLOAT weights[nWeights];
 Matrix4x4 matrixOffset;
}


Frame Frame_SCENE_ROOT {

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
 }

 Frame Frame1_Mark {

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }

  Mesh Mesh_Mark {
   32;
   0.000000;-0.050000;0.000000;,
   0.025000;-0.043301;0.000000;,
   0.012500;-0.043301;0.021651;,
   -0.012500;-0.043301;0.021651;,
   -0.025000;-0.043301;0.000000;,
   -0.012500;-0.043301;-0.021651;,
   0.012500;-0.043301;-0.021651;,
   0.043301;-0.025000;0.000000;,
   0.021651;-0.025000;0.037500;,
   -0.021651;-0.025000;0.037500;,
   -0.043301;-0.025000;0.000000;,
   -0.021651;-0.025000;-0.037500;,
   0.021651;-0.025000;-0.037500;,
   0.050000;0.000000;0.000000;,
   0.025000;0.000000;0.043301;,
   -0.025000;0.000000;0.043301;,
   -0.050000;0.000000;0.000000;,
   -0.025000;0.000000;-0.043301;,
   0.025000;0.000000;-0.043301;,
   0.043301;0.025000;0.000000;,
   0.021651;0.025000;0.037500;,
   -0.021651;0.025000;0.037500;,
   -0.043301;0.025000;0.000000;,
   -0.021651;0.025000;-0.037500;,
   0.021651;0.025000;-0.037500;,
   0.025000;0.043301;0.000000;,
   0.012500;0.043301;0.021651;,
   -0.012500;0.043301;0.021651;,
   -0.025000;0.043301;0.000000;,
   -0.012500;0.043301;-0.021651;,
   0.012500;0.043301;-0.021651;,
   0.000000;0.050000;0.000000;;
   36;
   3;30,31,25;,
   3;0,2,3;,
   3;0,3,4;,
   3;0,4,5;,
   3;0,5,6;,
   3;0,6,1;,
   3;29,28,31;,
   3;28,27,31;,
   3;26,31,27;,
   3;25,31,26;,
   3;29,31,30;,
   3;0,1,2;,
   4;9,8,14,15;,
   4;10,9,15,16;,
   4;11,10,16,17;,
   4;12,11,17,18;,
   4;7,13,14,8;,
   4;6,12,7,1;,
   4;4,3,9,10;,
   4;5,4,10,11;,
   4;6,5,11,12;,
   4;17,16,22,23;,
   4;18,17,23,24;,
   4;18,24,19,13;,
   4;19,25,26,20;,
   4;21,20,26,27;,
   4;22,21,27,28;,
   4;23,22,28,29;,
   4;24,23,29,30;,
   4;24,30,25,19;,
   4;3,2,8,9;,
   4;1,7,8,2;,
   4;15,14,20,21;,
   4;16,15,21,22;,
   4;12,18,13,7;,
   4;13,19,20,14;;

   MeshNormals {
    32;
    0.000000;-1.000000;0.000000;,
    0.492322;-0.870413;0.000000;,
    0.246161;-0.870413;0.426363;,
    -0.246161;-0.870413;0.426363;,
    -0.492322;-0.870413;0.000000;,
    -0.246161;-0.870413;-0.426363;,
    0.246161;-0.870413;-0.426363;,
    0.862093;-0.506749;0.000000;,
    0.431047;-0.506749;0.746595;,
    -0.431047;-0.506749;0.746595;,
    -0.862093;-0.506749;0.000000;,
    -0.431047;-0.506749;-0.746595;,
    0.431047;-0.506749;-0.746595;,
    1.000000;0.000000;-0.000000;,
    0.500000;-0.000000;0.866025;,
    -0.500000;-0.000000;0.866025;,
    -1.000000;0.000000;-0.000000;,
    -0.500000;0.000000;-0.866025;,
    0.500000;0.000000;-0.866025;,
    0.862093;0.506749;0.000000;,
    0.431047;0.506749;0.746595;,
    -0.431047;0.506749;0.746595;,
    -0.862093;0.506749;0.000000;,
    -0.431047;0.506749;-0.746595;,
    0.431047;0.506749;-0.746595;,
    0.492322;0.870413;0.000000;,
    0.246161;0.870413;0.426363;,
    -0.246161;0.870413;0.426363;,
    -0.492322;0.870413;0.000000;,
    -0.246161;0.870413;-0.426363;,
    0.246161;0.870413;-0.426363;,
    0.000000;1.000000;-0.000000;;
    36;
    3;30,31,25;,
    3;0,2,3;,
    3;0,3,4;,
    3;0,4,5;,
    3;0,5,6;,
    3;0,6,1;,
    3;29,28,31;,
    3;28,27,31;,
    3;26,31,27;,
    3;25,31,26;,
    3;29,31,30;,
    3;0,1,2;,
    4;9,8,14,15;,
    4;10,9,15,16;,
    4;11,10,16,17;,
    4;12,11,17,18;,
    4;7,13,14,8;,
    4;6,12,7,1;,
    4;4,3,9,10;,
    4;5,4,10,11;,
    4;6,5,11,12;,
    4;17,16,22,23;,
    4;18,17,23,24;,
    4;18,24,19,13;,
    4;19,25,26,20;,
    4;21,20,26,27;,
    4;22,21,27,28;,
    4;23,22,28,29;,
    4;24,23,29,30;,
    4;24,30,25,19;,
    4;3,2,8,9;,
    4;1,7,8,2;,
    4;15,14,20,21;,
    4;16,15,21,22;,
    4;12,18,13,7;,
    4;13,19,20,14;;
   }

   MeshMaterialList {
    1;
    36;
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0;

    Material {
     1.000000;0.000000;0.000000;1.000000;;
     51.200001;
     0.000000;0.000000;0.000000;;
     0.000000;0.000000;0.000000;;
    }
   }
  }
 }
}
