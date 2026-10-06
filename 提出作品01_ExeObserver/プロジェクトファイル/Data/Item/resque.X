xof 0302txt 0064
// LWS2X generated file 
// (LWS2X (C)1996-1998 A.Koizuka/Bio_100%  mailto:koizuka@ss.iij4u.or.jp )
// http://www.ss.iij4u.or.jp/~koizuka/

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

Header {
 1;
 0;
 1;
}

Material Matresque {
 1.000000;1.000000;1.000000;1.000000;;
 21.333333;
 0.000000;0.000000;0.000000;;
 0.000000;0.000000;0.000000;;
}
Material Matcross {
 1.000000;0.000000;0.000000;1.000000;;
 21.333333;
 0.000000;0.000000;0.000000;;
 0.000000;0.000000;0.000000;;
}
Mesh {
 40;
 -0.100000;-0.048912;-0.100000;,
 -0.100000;0.201088;0.300000;,
 -0.100000;0.201088;-0.100000;,
 -0.100000;-0.048912;0.300000;,
 0.200000;-0.048912;-0.100000;,
 0.200000;0.201088;0.300000;,
 0.200000;0.201088;-0.100000;,
 0.200000;-0.048912;0.300000;,
 -0.100000;0.126257;-0.100000;,
 -0.100000;0.125257;-0.100000;,
 -0.100000;0.126257;0.300000;,
 -0.100000;0.125257;0.300000;,
 0.200000;0.126257;-0.100000;,
 0.200000;0.125257;-0.100000;,
 0.200000;0.126257;0.300000;,
 0.200000;0.125257;0.300000;,
 -0.102236;0.067302;0.057512;,
 0.203053;0.067302;0.057512;,
 0.203053;0.067302;0.087821;,
 -0.102236;0.067302;0.087821;,
 -0.102236;0.036623;0.087082;,
 0.203053;0.036623;0.087082;,
 0.203053;0.036623;0.057512;,
 -0.102236;0.036623;0.057512;,
 -0.102236;0.097241;0.087451;,
 0.203053;0.097241;0.087451;,
 0.203053;0.006683;0.087082;,
 -0.102236;0.006683;0.087082;,
 0.203053;0.006683;0.117391;,
 -0.102236;0.006683;0.117391;,
 0.203053;0.097241;0.117391;,
 -0.102236;0.097241;0.117391;,
 -0.102236;0.066932;0.117391;,
 0.203053;0.066932;0.117391;,
 0.203053;0.066563;0.146961;,
 -0.102236;0.066563;0.146961;,
 0.203053;0.036623;0.146961;,
 -0.102236;0.036623;0.146961;,
 -0.102236;0.036623;0.117391;,
 0.203053;0.036623;0.117391;;

 16;
 4;3,11,9,0;,
 4;0,9,13,4;,
 4;3,7,15,11;,
 4;4,13,15,7;,
 4;7,3,0,4;,
 4;1,2,8,10;,
 4;2,6,12,8;,
 4;1,10,14,5;,
 4;5,14,12,6;,
 4;6,2,1,5;,
 4;32,38,37,35;,
 4;24,27,29,31;,
 4;16,23,20,19;,
 4;33,34,36,39;,
 4;25,30,28,26;,
 4;17,18,21,22;;

 MeshMaterialList {
  2;
  16;
  0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1;;
  {Matresque}
  {Matcross}
 }
 MeshNormals {
  16;
  -1.000000;0.000000;0.000000;,
  0.000000;0.000000;-1.000000;,
  0.000000;0.000000;1.000000;,
  1.000000;0.000000;0.000000;,
  0.000000;-1.000000;0.000000;,
  -1.000000;0.000000;0.000000;,
  0.000000;0.000000;-1.000000;,
  0.000000;0.000000;1.000000;,
  1.000000;0.000000;0.000000;,
  0.000000;1.000000;0.000000;,
  -1.000000;0.000000;0.000000;,
  -1.000000;0.000000;0.000000;,
  -1.000000;0.000000;0.000000;,
  1.000000;0.000000;0.000000;,
  1.000000;0.000000;0.000000;,
  1.000000;0.000000;0.000000;;

  16;
  4;0,0,0,0;,
  4;1,1,1,1;,
  4;2,2,2,2;,
  4;3,3,3,3;,
  4;4,4,4,4;,
  4;5,5,5,5;,
  4;6,6,6,6;,
  4;7,7,7,7;,
  4;8,8,8,8;,
  4;9,9,9,9;,
  4;10,10,10,10;,
  4;11,11,11,11;,
  4;12,12,12,12;,
  4;13,13,13,13;,
  4;14,14,14,14;,
  4;15,15,15,15;;
 }
}
