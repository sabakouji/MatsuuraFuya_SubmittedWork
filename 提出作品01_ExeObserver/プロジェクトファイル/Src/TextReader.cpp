#include "TextReader.h"
#include <fstream>
#include <assert.h>
#include <tchar.h>
#include <codecvt>
#include "MyImgui.h"

namespace {
	const int BUFSIZE = 2048;
};

TextReader::TextReader(tstring filename)
{
	FILE* fp;
	TCHAR	inputBuffer[BUFSIZE];
	TCHAR*  s;
	char	inputBufferU8[BUFSIZE];
	bool u8 = false, ret = false; 
	errno_t error;

	all.clear();

	// --------------------------------------------
	//error = _tfopen_s(&fp, filename.c_str(), _T("r, ccs = UNICODE"));	// サーバースクリプト読み込み(Unicodeで読み込み)
	error = _tfopen_s(&fp, filename.c_str(), _T("r"));	// サーバースクリプト読み込み
	if (error == 0 )
	{
		// ファイルの先頭のBOMをチェックする
		char bom[4];
		fgets(bom, 3 + 1, fp);
		if ((unsigned char)bom[0] == 0xEF && (unsigned char)bom[1] == 0xBB && (unsigned char)bom[2] == 0xBF)
		{
			u8 = true;		// UTF-8である
		}
		else {
			fseek(fp, 0, SEEK_SET);	  // シーク位置を先頭に戻す
		}

		// 読み込む
		while (true)
		{
			if (u8)
			{
				// UTF-8
				if (fgets(inputBufferU8, BUFSIZE, fp) == nullptr)  break;   // ファイルの最後に達するまで一行づつ読み込む
				// TCHARにコンバート
				MyImgui::ConvertUTF8ToTCHAR(inputBufferU8, inputBuffer);
			}
			else {
				// TCHAR
				if (_fgetts(inputBuffer, BUFSIZE, fp) == nullptr)  break;   // ファイルの最後に達するまで一行づつ読み込む
			}
			ret = true;

			s = _tcsstr(inputBuffer, _T("//"));      // コメント（//）の後の文字列を削除する
			if (s != nullptr)
			{
				*s = _T('\0');
			}
			s = _tcsstr(inputBuffer, _T("\n"));      // 文字列の最後の\nを削除する
			if (s != nullptr)
			{
				*s = _T('\0');
			}
			for (int i = 0; i < _tcslen(inputBuffer); i++)		// 文字 " を削除する
			{
				if (inputBuffer[i] == _T('"'))	 inputBuffer[i] = _T(' ');
			}

			if (_tcscmp(inputBuffer, _T("")) == 0 )  continue;	  // 文字列なしの行を読み飛ばす

			int ii;
			for (ii = 0; ii < _tcslen(inputBuffer); ii++)  // 空白行を読み飛ばす
			{
				if (inputBuffer[ii] != _T(' ') && inputBuffer[ii] != _T('\t')) break;
			}
			if( ii >= _tcslen(inputBuffer))	  continue;


			// 行内を , 空白 タブ で切り分ける
			LINEREC lineRecord;
			TCHAR str[BUFSIZE] = {};
			int slen = 0;
			bool spt;

			for (int n = 0; n < _tcslen(inputBuffer); n++)
			{
				spt = false;
				while (inputBuffer[n] == _T(' ') || inputBuffer[n] == _T('\t'))	   // 連続する空白とタブを１つにまとめる
				{
					spt = true;    // 空白かタブがあったとき
					n++;
				}
				if ( spt && inputBuffer[n] != _T(','))	// 空白かタブがあった後「 ，」以外が来たとき
				{
					if (slen > 0)		// 事前に文字列があったとき
					{
						str[slen] = _T('\0');
						lineRecord.record.emplace_back(str);	// 1項目ため込む
						slen = 0;
					}
					str[slen] = inputBuffer[n];	   // 1文字をstrにため込む
					slen++;
				}else if (inputBuffer[n] == _T(','))		// 「 ，」が来たとき
				{
					str[slen] = _T('\0');
					lineRecord.record.emplace_back(str);	// 無条件で1項目ため込む
					slen = 0;
				}
				else {
					str[slen] = inputBuffer[n];	   // 1文字をstrにため込む
					slen++;
				}
			}
			if (slen > 0)
			{
				str[slen] = _T('\0');
				lineRecord.record.emplace_back(str);	// 最後の1項目ため込む
				slen = 0;
			}
			all.emplace_back(lineRecord);		// 1行ため込む
		}
		fclose(fp);
	}
	else {
		MessageBox(0, filename.c_str(), _T("TextReader() テキスト読み込み処理　テキストファイルがありません"), MB_OK);
		assert(false);
	}
}

TextReader::~TextReader()
{
	for (auto rec : all)
		rec.record.clear();
	all.clear();
}

unsigned int TextReader::GetLines()
{
	return (unsigned int)all.size();
}

unsigned int TextReader::GetColumns(unsigned int line)
{

	if (line >= GetLines())
	{
		MessageBox(0, 0, _T("GetColumns() 不正な行数指定です"), MB_OK);
		assert(false);
	}
	return (unsigned int)all[line].record.size();
}

tstring TextReader::GetString(unsigned int line, unsigned int column)
{
	if (line >= GetLines())
	{
		MessageBox(0, 0, _T("GetString() 不正な行数指定です"), MB_OK);
		assert(false);
	}
	if (column >= GetColumns(line))
		return _T("");
	return all[line].record[column];
}

int TextReader::GetInt(unsigned int line, unsigned int column)
{
	std::string str = GetString(line, column);
	return std::stoi(str);
}

float TextReader::GetFloat(unsigned int line, unsigned int column)
{
	std::string str = GetString(line, column);
	return std::stof(str);
}
