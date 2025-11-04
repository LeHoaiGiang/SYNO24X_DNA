#include "amidite.h"
#include <QString>
#include <QVector>
#include <QStack>
#include "qmessagebox.h"
#include <QMessageBox>
/*
 * 25-04-2025 update function 6 amidite floatting
 *
 *
 * */
#define PRINT_DEBUG
amidite::amidite()
{

}

void amidite::setpath(QString Path)
{
    amiditePath = Path;
}
// floatting amidite ========================================================== for SYNO24X update 21.08.2024
void amidite::saveAmiditeFloatting(const QString& amidite_f1, const QString& amidite_f2) {
    // Tạo một đối tượng JSON
    QJsonObject jsonObject;
    jsonObject["amidite_f1"] = amidite_f1;
    jsonObject["amidite_f2"] = amidite_f2;

    // Tạo tài liệu JSON từ đối tượng JSON
    QJsonDocument jsonDoc(jsonObject);

    // Mở tệp để ghi
    QFile file(amiditePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file to save JSON.");
        return;
    }

    // Ghi tài liệu JSON vào tệp
    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();
    saveToJson();
}
//=============================================================================== for SYNO24X update 21.08.2024
void amidite::readAmiditeFloatting(QString& amidite_f1, QString& amidite_f2) {
    // Mở tệp để đọc
    QFile file(amiditePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file to read JSON.");
        return;
    }

    // Đọc nội dung tệp và chuyển đổi thành tài liệu JSON
    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));
    file.close();

    // Kiểm tra nếu tài liệu JSON hợp lệ
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qWarning("Invalid JSON document.");
        return;
    }

    // Lấy đối tượng JSON và đọc các giá trị amidite_f1 và amidite_f2
    QJsonObject jsonObject = jsonDoc.object();
    amidite_f1 = jsonObject["amidite_f1"].toString();
    amidite_f2 = jsonObject["amidite_f2"].toString();
    loadFromJson();
}

// Thêm vào class ChemicalManager
void amidite::saveToJson() {
    QJsonObject jsonObject;

    // Lưu tên lọ 5 đến 10
    QJsonArray bottleArray;
    for (int i = 0; i < 6; ++i) {
        QString bottleName = bottleNameInputs[i]->text().trimmed();
        if (!bottleName.isEmpty()) {
            bottleArray.append(bottleName);
        }
        qDebug() << "bottleName :" << i << " : " << bottleName ;
        bottleNamesList[i] = bottleArray[i].toString();
    }
    jsonObject["bottles"] = bottleArray;

    // Lưu vào file JSON
    QFile file("chemicals.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(jsonObject).toJson());
        file.close();
        //QMessageBox::information(this, "Thông báo", "Lưu tên hóa chất thành công!");
    } else {
        // QMessageBox::critical(this, "Lỗi", "Không thể lưu file JSON.");
    }
    // Nối bottleNamesList vào bottleNamesListFull
    bottleNamesListFull.clear();
    bottleNamesListFull.append(bottleNamesFirstList); // thêm từ floatting amidte
    bottleNamesListFull.append(bottleNamesList); // thêm từ floatting amidte
    bottleNamesListFull.append(bottleNamesLastList); // thêm các hóa chất dùng chung
    bottleNamesListAmidite.clear();
    bottleNamesListAmidite.append(bottleNamesFirstList); // thêm từ  amidte cố định
    bottleNamesListAmidite.append(bottleNamesList); // thêm từ floatting amidte
}

void amidite::loadFromJson() {
    QFile file("chemicals.json");
    if (!file.exists()) {
        return; // Không có file JSON thì không làm gì cả
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject jsonObject = doc.object();
            QJsonArray bottleArray = jsonObject["bottles"].toArray();
            // Điền tên lọ từ JSON
            for (int i = 0; i < bottleArray.size() && i < 6; ++i) {
                bottleNameInputs[i]->setText(bottleArray[i].toString());
                bottleNamesList[i] = bottleArray[i].toString();
            }
        }
    }
    // Nối bottleNamesList vào bottleNamesListFull
    bottleNamesListFull.clear();
    bottleNamesListFull.append(bottleNamesFirstList); // thêm từ floatting amidte
    bottleNamesListFull.append(bottleNamesList); // thêm từ floatting amidte
    bottleNamesListFull.append(bottleNamesLastList); // thêm các hóa chất dùng chung
    // dùng cho việc lựa chọn floatting amidite
    bottleNamesListAmidite.clear();
    bottleNamesListAmidite.append(bottleNamesFirstList); // thêm từ  amidte cố định
    bottleNamesListAmidite.append(bottleNamesList); // thêm từ floatting amidte
}

void amidite :: amiditeSyno24XGetSequence(global_var_t* global_var)
{
    QByteArray  lineData;
    QString transformedAmiditeStr;
    global_var->signal_status_oligo.u16_max_sequence_amidite_setting = 0;
    //    QString source = "aTgC{example}ATGC[FAN](S){remove}[HEX][HEX]";
    //    transformed = transformString(source, sequence_floatting_1, sequence_floatting_2);
    //    qDebug() << "Original:" << source;
    //    qDebug() << "Transformed:" << transformed;
    //    qDebug() << "Length of transformed string:" << transformed.length();
    //=================================================================================================================================
    //QString source_linkage = "CCC(S)T(S)CC(S)A(S)A";
    QString String_OX;
    //    String_OX = transformString_Ox(source, sequence_floatting_1, sequence_floatting_2);
    //    qDebug() << "source:" << source;
    //    qDebug() << "String_OX:" << String_OX;
    //QString source_linkage = "CCC(S)TCCA(S)A(S)TCC(S)C";
    QString result = reverseStringWithParentheses(String_OX);
    //qDebug() << "Reversed String:" << result;
    // Liệt kê các ký tự và liên kết
    QVector<QChar> characters(128, CHEMICAL_SUBTANCE_EMPTY);  // Khởi tạo 128 phần tử với giá trị mặc định là 'CHEMICAL_SUBTANCE_EMPTY'
    QVector<QChar> linkages(128, '1');    // Khởi tạo 128 phần tử với giá trị mặc định là '1'
    //determineCharactersAndLinkages(result, characters, linkages);
    //In kết quả
    for(uint8_t well = 0; well < MAX_WELL_AMIDITE ; well++)
    {
        transformedAmiditeStr = "";
        result = "";
        String_OX="";
        characters.clear();
        linkages.clear();
        QString sequence = global_var->amidite_well[well].string_sequence;

        // Kiểm tra nếu chuỗi rỗng thì khong cần xử lý
        if (sequence.isEmpty()) {
            //qDebug() << "Well:" << well << "has an empty sequence. Setting default values.";
            global_var->amidite_well[well].sequenceLength = 0;
            for (int i = 0; i < 127; i++) {
                //characters[i] = CHEMICAL_SUBTANCE_EMPTY;  // Đặt toàn bộ phần tử characters là 'A'
                global_var->amidite_well[well].u8_sequence[i] = CHEMICAL_SUBTANCE_EMPTY; // biến này mới thực là data trong chương trình chính nè
                global_var->amidite_well[well].linkages[i] = 1;
                //linkages[i] = 1;    // Đặt toàn bộ phần tử linkages là '1'
            }
            //continue; // Bỏ qua các bước xử lý còn lại
        }
        else
        {
            // lấy chuỗi chứa kí tự của liên kết PO-PS
            String_OX = transformString_OxFloatting(global_var->amidite_well[well].string_sequence, bottleNamesList);
            // Lấy Sequence đã được xử lý
            transformedAmiditeStr = transformString_SequenceFloatting(global_var->amidite_well[well].string_sequence, bottleNamesList);
            // transformedAmiditeStr = transformString(global_var->amidite_well[well].string_sequence, sequence_floatting_1, sequence_floatting_2);
            // String_OX = transformString_Ox(global_var->amidite_well[well].string_sequence, sequence_floatting_1, sequence_floatting_2);
            result = reverseStringWithParentheses(String_OX);
            // Gán giá trị cho toàn bộ các phần tử từ 0 đến 127
            for (int i = 0; i < 127; i++) {
                characters[i] = CHEMICAL_SUBTANCE_EMPTY;
                global_var->amidite_well[well].u8_sequence[i] = CHEMICAL_SUBTANCE_EMPTY; // biến này mới thực là data trong chương trình chính nè
                global_var->amidite_well[well].linkages[i] = 1;
                linkages[i] = 1;    // Đặt toàn bộ phần tử linkages là '1'
            }
            determineCharactersAndLinkages(result, characters, linkages);
            // mở debug chỗ này ra
#ifdef PRINT_DEBUG
            qDebug() << "Well :" << well<< "Transformed:" << transformedAmiditeStr<< "source:"
                     << global_var->amidite_well[well].string_sequence <<"Reversed String:"
                     << result << "Length of sequence:" << transformedAmiditeStr.length();;
            //debug trinh tự sau khi chuyển đổi mở comment này
            for (int i = 0; i < characters.size(); ++i) {
                qDebug() << "amidite:" << characters[i] << "ox:" << linkages[i];
            }
#endif
            global_var->amidite_well[well].sequenceLength = transformedAmiditeStr.length();

            std::reverse(transformedAmiditeStr.begin(), transformedAmiditeStr.end()); // dong nay reverd data
            lineData = transformedAmiditeStr.toLocal8Bit();
            // Lấy từ chuỗi gốc
            if(transformedAmiditeStr.length() > 0)
            {
                global_var->amidite_well[well].FirstSequence = char2quint8_Amidite(transformedAmiditeStr.toLocal8Bit()[0]);
                global_var->amidite_well[well].lastSequence = char2quint8_Amidite(transformedAmiditeStr.toLocal8Bit()[transformedAmiditeStr.length() - 1]);
                global_var->amidite_well[well].LastbaseinWellCount = transformedAmiditeStr.length() - 1; // lấy counter sequence
                qDebug() << "Well :" << well<< "FirstSequence:" << global_var->amidite_well[well].FirstSequence
                         << "lastSequence:"<< global_var->amidite_well[well].lastSequence
                         << "Last base in WellCount:"<< global_var->amidite_well[well].LastbaseinWellCount;
            }
            for(uint8_t u8_sequence = 0; u8_sequence < MAX_SEQUENCE_OF_WELL; u8_sequence++)
            {
                if(transformedAmiditeStr.length() > 0)
                {
                    global_var->amidite_well[well].u8_sequence[u8_sequence] = char2quint8_Amidite(lineData[u8_sequence]);
                    //qDebug() << "sequence "<<global_var->amidite_well[well].u8_sequence[u8_sequence];
                    if(linkages[u8_sequence] == 'S')
                    {
                        global_var->amidite_well[well].linkages[u8_sequence] = 2;
                        //mở debug xem sequence
                        //qDebug() << "Sequence:" << u8_sequence<< "Linkage : " << global_var->amidite_well[well].linkages[u8_sequence];
                    }
                    else
                    {
                        global_var->amidite_well[well].linkages[u8_sequence] = 1;
                        // mở debug xem oxidation sequence 1 là OX1 || 2 LÀ OX2
                        // qDebug() << "Sequence:" << u8_sequence << "Linkage : " << global_var->amidite_well[well].linkages[u8_sequence];
                    }
                    //            if(global_var->signal_status_oligo.u16_max_sequence_amidite_setting <  transformedAmiditeStr.length())
                    //            {
                    //                global_var->signal_status_oligo.u16_max_sequence_amidite_setting = transformedAmiditeStr.length();
                    //                qDebug() << "Max length sequence amidite:" << global_var->signal_status_oligo.u16_max_sequence_amidite_setting;
                    //            }
                    if(u8_sequence == 0)
                    {
                        global_var->amidite_well[well].FirstSequence = char2quint8_Amidite(lineData[u8_sequence]);
                        qDebug() << "FirstSequence sequence amidite:" << global_var->amidite_well[well].FirstSequence;
                    }
                    else
                    {
                        if(u8_sequence == (transformedAmiditeStr.length() - 1))
                        {
                            global_var->amidite_well[well].lastSequence = char2quint8_Amidite(lineData[u8_sequence]);
                            qDebug() << "lastSequence sequence amidite:" << global_var->amidite_well[well].lastSequence;
                        }
                    }

                }
            }
        }

        if(global_var->signal_status_oligo.u16_max_sequence_amidite_setting <  transformedAmiditeStr.length())
        {
            global_var->signal_status_oligo.u16_max_sequence_amidite_setting = transformedAmiditeStr.length();
            //qDebug() << "==================== Max length sequence amidite:" << global_var->signal_status_oligo.u16_max_sequence_amidite_setting;
#ifdef PRINT_DEBUG
            std::reverse(transformedAmiditeStr.begin(), transformedAmiditeStr.end()); // dong nay reverd data
#endif
        }

    } // for 24 cột
#ifdef PRINT_DEBUG
    qDebug() << "==================== Max length sequence amidite:" << global_var->signal_status_oligo.u16_max_sequence_amidite_setting;
#endif

}


// Return value to fill chemical amiite
quint8 amidite:: char2quint8_Amidite(char DataIn)
{
    switch (DataIn)
    {
    case 'A':
    {
        return AMD_A;
        break;
    }
    case 'T':
    {
        return AMD_T;
        break;
    }
    case 'G':
    {
        return AMD_G;
        break;
    }
    case 'C':
    {
        return AMD_C;
        break;
    }
    case 'a':
    {
        return AMD_a;
        break;
    }
    case 't':
    {
        return AMD_t;
        break;
    }
    case 'g':
    {
        return AMD_g;
        break;
    }
    case 'c':
    {
        return AMD_c;
        break;
    }

    case 'I':
    {
        return AMD_I;//4
        break;
    }
    case 'U':
    {
        return AMD_U;
        break;
    }
    case 'Y':
    {
        return AMD_Y;
        break;
    }
    case 'R':
    {
        return AMD_R;
        break;
    }
    case 'W':
    {
        return AMD_W;
        break;
    }
    case 'S':
    {
        return AMD_S;
        break;
    }
    case 'K':
    {
        return AMD_K;
        break;
    }
    case 'M':
    {
        return AMD_M;
        break;
    }
    case 'D':
    {
        return AMD_D;
        break;
    }

    case 'V':
    {
        return AMD_V;
        break;
    }
    case 'N':
    {
        return AMD_N;
        break;
    }
    case 'X': // X TƯƠNG TỰ  N
    {
        return AMD_N;
        break;
    }
    case 'H':
    {
        return AMD_H;
        break;
    }
    case 'B':
    {
        return AMD_B;
        break;
    }
    default:
    {
        return CHEMICAL_SUBTANCE_EMPTY; // neu khong phai thi return 0x7F
    }
    }
}



QList<CharacterLinkage> amidite:: convertToLinkages(const QString &input) {
    QList<CharacterLinkage> linkages;
    QString reversedInput = input; // Create a copy of the input string
    std::reverse(reversedInput.begin(), reversedInput.end()); // Reverse the string
    int length = reversedInput.length();
    QChar lastCharacter; // Last character before (S)
    for (int i = 0; i < length; ++i) {
        QChar ch = reversedInput[i];
        // Check if the current character is part of (S)
        if (ch == ')' && i - 2 >= 0 && reversedInput[i - 1] == 'S' && reversedInput[i - 2] == '(') {
            // Set linkage for the last character before (S)
            if (!lastCharacter.isNull() && lastCharacter != ' ') {
                linkages.append({lastCharacter, S}); // Last character gets S linkage
                lastCharacter = QChar(); // Reset last character
            }
            // Skip next characters as they are part of the linkage
            i += 2;
            continue;
        } else if (ch == '(' && i + 2 < length && reversedInput[i + 1] == 'S' && reversedInput[i + 2] == ')') {
            // Set linkage for the last character before (S)
            if (!lastCharacter.isNull() && lastCharacter != ' ') {
                linkages.append({lastCharacter, S}); // Last character gets S linkage
                lastCharacter = QChar(); // Reset last character
            }
            // Skip next characters as they are part of the linkage
            i += 2;
            continue;
        } else if (ch == '(' || ch == ')') {
            // Skip parentheses
            continue;
        } else {
            // Handle characters outside (S)
            if (!lastCharacter.isNull() && lastCharacter != ' ') {
                linkages.append({lastCharacter, O}); // Add the last character with O linkage
            }
            lastCharacter = ch;
        }
    }
    // Add the last character with default linkage O if not processed
    if (!lastCharacter.isNull() && lastCharacter != ' ') {
        linkages.append({lastCharacter, O});
    }
    // Ensure the array has exactly 9 elements
    while (linkages.size() < 9) {
        linkages.append({' ', O}); // Add spaces with default linkage if needed
    }
    return linkages;
}

void amidite:: printLinkages(const QList<CharacterLinkage> &linkages) {
#ifdef PRINT_DEBUG
    for (const auto &linkage : linkages) {
        qDebug() << "Character:" << linkage.character << ", Linkage:" << (linkage.linkage == O ? "O" : "S");

    }
#endif

}
/* Xử lý quy tắc 3' 5' đảo ngược chuỗi nên cần đảo ngược dấu ngoặc trước khi xử lý trình tự và Link PO-PS
 * reverseStringWithParentheses input : "TT(S)TTU"
 * reverseStringWithParentheses output : "UTT(S)TT"
*/

QString  amidite::reverseStringWithParentheses(const QString& input) {
#ifdef PRINT_DEBUG
    qDebug() << "reverseStringWithParentheses input :" << input;
#endif
    QString reversed;
    QStack<QChar> bracketStack;

    for (int i = input.length() - 1; i >= 0; --i) {
        QChar ch = input.at(i);

        // Nếu gặp dấu đóng ngoặc, đẩy vào stack
        if (ch == ')') {
            reversed += '(';
        }
        // Nếu gặp dấu mở ngoặc, lấy dấu đóng ngoặc tương ứng từ stack và thêm vào chuỗi kết quả
        else if (ch == '(') {
            reversed += ')';
        }
        // Các ký tự khác, thêm trực tiếp vào chuỗi kết quả
        else {
            reversed += ch;
        }
    }

    // Thêm các dấu đóng ngoặc còn lại trong stack (nếu có) vào đầu chuỗi kết quả
    while (!bracketStack.isEmpty()) {
        reversed += bracketStack.pop();
    }
#ifdef PRINT_DEBUG
    qDebug() << "reverseStringWithParentheses output :" << reversed;
#endif
    return reversed;
}
/*
 * Xác định trình tự và liên kết PO-PS
 * xử lý và xuất trực tiếp ra vector giá trị của trình tự và liên kết PO-PS
 *  determineCharactersAndLinkages input : "UTT(S)TT"
 *  determineCharactersAndLinkages characters : QVector('U', 'T', 'T', 'T', 'T')
 *  determineCharactersAndLinkages linkages : QVector('O', 'O', 'S', 'O', 'O')
 *
*/
void amidite::determineCharactersAndLinkages(const QString& sequence, QVector<QChar>& characters, QVector<QChar>& linkages) {
#ifdef PRINT_DEBUG
    qDebug() << "determineCharactersAndLinkages input :" << sequence;
#endif
    QString cleanedSequence;
    QChar currentLinkage = 'O'; // Liên kết mặc định
    //QMap<QChar, QChar> lastLinkageMap;

    for (int i = 0; i < sequence.length(); ++i) {
        QChar c = sequence[i];

        if (c == '(' || c == ')') {
            // Bỏ qua dấu ngoặc đơn
            continue;
        }

        if (c == 'S') {
            // Cập nhật liên kết thành 'S' cho ký tự tiếp theo
            currentLinkage = 'S';
        } else {
            if (!cleanedSequence.isEmpty()) {
                // Thêm ký tự trước đó vào danh sách và gán liên kết của nó
                linkages.append(currentLinkage);
                characters.append(cleanedSequence.back());
            }
            // Cập nhật ký tự và liên kết hiện tại
            cleanedSequence.append(c);
            currentLinkage = 'O'; // Đặt lại liên kết mặc định cho ký tự tiếp theo
        }
    }

    // Xử lý ký tự cuối cùng trong cleanedSequence
    if (!cleanedSequence.isEmpty()) {
        linkages.append(currentLinkage);
        characters.append(cleanedSequence.back());
    }
#ifdef PRINT_DEBUG
    qDebug() << "determineCharactersAndLinkages characters :" << characters;
    qDebug() << "determineCharactersAndLinkages linkages :" << linkages;
#endif
}

/*
 *  25-04-2025
 *  Hàm này xử lý chuỗi người dùng nhập vào mục đich xóa các kí tự không liên quan sau đó chuyển các modify amidte về dạng dạng bình thương macro trong máy để lát nữa xử lý sau
 *  ví dụ sau:
 *  floatting 1 đang là FAM
 *  floating 2 đang là HEX
 *  input : TT(S)TT[HEX]{remove}
 *  output : TTTTU
 */

QString amidite:: transformString(const QString &input, const QString &editTextFloating_1, const QString &editTextFloating_2) {

#ifdef PRINT_DEBUG
    qDebug() << "transformString input :" << input;
#endif
    QString result;
    bool inBraces = false;
    bool inSquareBrackets = false;
    bool inParentheses = false;
    QString temp;

    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i];

        if (ch == '{') {
            inBraces = true;
            temp.clear(); // Clear temp if needed
        } else if (ch == '}') {
            inBraces = false;
            temp.clear(); // Discard anything collected within {}
        } else if (ch == '[' && !inBraces && !inParentheses) {
            inSquareBrackets = true;
            temp.clear();
        } else if (ch == ']' && !inBraces && !inParentheses) {
            inSquareBrackets = false;
            if (temp == editTextFloating_1) {
                result += 'I';
            } else if (temp == editTextFloating_2) {
                result += 'U';
            } else {
                result += ' ';  // Replace with space if it doesn't match
            }
        } else if (ch == '(') {
            inParentheses = true;
        } else if (ch == ')') {
            inParentheses = false;
        } else if (inBraces || inParentheses) {
            // Skip characters inside curly braces or parentheses
            continue;
        } else if (inSquareBrackets) {
            // Collect characters inside square brackets
            temp += ch;
        } else {
            result += ch;
        }
    }
#ifdef PRINT_DEBUG
    qDebug() << "transformString Output :" << result;
#endif
    return result;
}


/*
 *  25-04-2025
 *  Hàm này xử lý chuỗi người dùng nhập vào mục đich xóa các kí tự không liên quan sau đó chuyển các modify amidte về dạng dạng bình thương macro trong máy để lát nữa xử lý sau
 *  hàm này sẽ giữ lại các trường dữ liệu (S) để tạo liên kết, xóa bỏ các kí hiệu trong dấu ngoặc nhọn
 *  ví dụ sau:
 *  floatting 1 đang là FAM
 *  floating 2 đang là HEX
 *  input : TT(S)TT[HEX]{remove}
 *  output :  TT(S)TTU
 */

QString amidite:: transformString_Ox(const QString &input, const QString &editTextFloating_1, const QString &editTextFloating_2) {
#ifdef PRINT_DEBUG
    qDebug() << "transformString_Ox input :" << input;
#endif
    QString result;
    bool inBraces = false;
    bool inSquareBrackets = false;
    QString temp;

    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i];

        if (ch == '{') {
            inBraces = true;
            temp.clear(); // Xóa tạm nếu cần
        } else if (ch == '}') {
            inBraces = false;
            temp.clear(); // Bỏ qua những gì thu thập trong {}
        } else if (ch == '[' && !inBraces) {
            inSquareBrackets = true;
            temp.clear();
        } else if (ch == ']' && !inBraces) {
            inSquareBrackets = false;
            if (temp == editTextFloating_1) {
                result += 'I';
            } else if (temp == editTextFloating_2) {
                result += 'U';
            } else {
                result += ' ';  // Replace with space if it doesn't match
            }
        } else if (inBraces) {
            // Bỏ qua các ký tự bên trong {}
            continue;
        } else if (inSquareBrackets) {
            // Thu thập các ký tự bên trong []
            temp += ch; // temp khi gặp kí tự [] sẽ lấy hết các phần tử trong ngoặc
        } else {
            result += ch;
        }
    }
#ifdef PRINT_DEBUG
    qDebug() << "transformString_Ox Output :" << result;
#endif
    return result;
}


/*
 *
 * *  25-04-2025
 *  Hàm này xử lý chuỗi người dùng nhập vào mục đich xóa các kí tự không liên quan sau đó chuyển các modify amidte về dạng dạng bình thương macro trong máy để lát nữa xử lý sau
 *  hàm này sẽ giữ lại các trường dữ liệu (S), hỗ trợ 6 Floatting AMIDITE
 *  transformString_Ox input : "TT(S)TT[HEX]{remove}[BHQ]"
 *  transformString_Ox Output : "TT(S)TTU "
 *
 *
 *
*/
QString amidite::transformString_OxFloatting(const QString &input, const QStringList &bottleNames) {
#ifdef PRINT_DEBUG
    qDebug() << "transformString_SequenceFloatting input :" << input;
#endif

    QString result;
    bool inBraces = false;
    bool inSquareBrackets = false;
    QString temp;

    // Tạo một map để ánh xạ tên lọ 5-10 thành ký tự tương ứng
    QMap<QString, QChar> bottleMap;
    for (int i = 0; i < bottleNames.size(); ++i) {
        if (i < 6) {
            bottleMap[bottleNames[i]] = "atgcIU"[i]; // Ánh xạ theo thứ tự: a, t, g, c, I, U
        }
    }

    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i];

        if (ch == '{') {
            inBraces = true;
            temp.clear(); // Xóa tạm nếu cần
        } else if (ch == '}') {
            inBraces = false;
            temp.clear(); // Bỏ qua những gì thu thập trong {}
        } else if (ch == '[' && !inBraces) {
            inSquareBrackets = true;
            temp.clear();
        } else if (ch == ']' && !inBraces) {
            inSquareBrackets = false;

            // Kiểm tra xem tên lọ có trong map không
            if (bottleMap.contains(temp)) {
                result += bottleMap[temp]; // Ánh xạ tên lọ thành ký tự tương ứng
            } else {
                result += ' '; // Replace with space if it doesn't match
            }
        } else if (inBraces) {
            // Bỏ qua các ký tự bên trong {}
            continue;
        } else if (inSquareBrackets) {
            // Thu thập các ký tự bên trong []
            temp += ch; // temp khi gặp kí tự [] sẽ lấy hết các phần tử trong ngoặc
        } else {
            result += ch;
        }
    }

#ifdef PRINT_DEBUG
    qDebug() << "transformString_SequenceFloatting Output :" << result;
#endif

    return result;
}


/* 25-04-2025
 *  xử lý string sang sequence
 *  transformString_SequenceFloatting input : "TT(S)TT[HEX]{remove}[BHQ]"
 *  transformString_SequenceFloatting Output : "TT(S)TTtg"
 *
*/
QString amidite::transformString_SequenceFloatting(const QString &input, const QStringList &bottleNames) {
#ifdef PRINT_DEBUG
    qDebug() << "transformStringSequnce input :" << input;
#endif

    QString result;
    bool inBraces = false;
    bool inSquareBrackets = false;
    bool inParentheses = false;
    QString temp;

    // Tạo một map để ánh xạ tên lọ 5-10 thành ký tự tương ứng
    QMap<QString, QChar> bottleMap;
    for (int i = 0; i < bottleNames.size(); ++i) {
        if (i < 6) {
            bottleMap[bottleNames[i]] = "atgcIU"[i]; // Ánh xạ theo thứ tự: a, t, g, c, I, U
        }
    }

    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i];

        if (ch == '{') {
            inBraces = true;
            temp.clear(); // Clear temp if needed
        } else if (ch == '}') {
            inBraces = false;
            temp.clear(); // Discard anything collected within {}
        } else if (ch == '[' && !inBraces && !inParentheses) {
            inSquareBrackets = true;
            temp.clear();
        } else if (ch == ']' && !inBraces && !inParentheses) {
            inSquareBrackets = false;

            // Kiểm tra xem tên lọ có trong map không
            if (bottleMap.contains(temp)) {
                result += bottleMap[temp]; // Ánh xạ tên lọ thành ký tự tương ứng
            } else {
                result += ' '; // Replace with space if it doesn't match
            }
        } else if (ch == '(') {
            inParentheses = true;
        } else if (ch == ')') {
            inParentheses = false;
        } else if (inBraces || inParentheses) {
            // Skip characters inside curly braces or parentheses
            continue;
        } else if (inSquareBrackets) {
            // Collect characters inside square brackets
            temp += ch;
        } else {
            result += ch;
        }
    }

#ifdef PRINT_DEBUG
    qDebug() << "transformStringSequnce Output :" << result;
#endif

    return result;
}
/*
 * 26-04-2025 check and warning sequence form user
 * kiểm tra trình tự người dùng nhập vào đã đúng với cài đặt hay chưa
 * nếu không đúng thì thông báo cho người dùng
*/
bool amidite::validateInput(const QString &input, const QStringList &bottleNamesList, QString &errorMessage) {
    errorMessage.clear(); // Clear the error message initially
    bool inBraces = false;
    bool inSquareBrackets = false;
    bool inParentheses = false;
    QString temp;

    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i];

        // Check if character is outside brackets
        if (!inBraces && !inSquareBrackets && !inParentheses) {
            // Only A, T, G, C are allowed outside brackets
            if (!QString("ATGCYRWSKMDVNXBH").contains(ch) && !QString("[]{}()").contains(ch)) {
                errorMessage = QString("Invalid character '%1' at position %2. Only 'A', 'T', 'G', 'C', or brackets are allowed outside brackets.").arg(ch).arg(i + 1);
                return false;
            }
        }

        // Handle curly braces {}
        if (ch == '{') {
            if (inBraces) {
                errorMessage = QString("Unclosed '{' bracket at position %1.").arg(i + 1);
                return false;
            }
            inBraces = true;
        } else if (ch == '}') {
            if (!inBraces) {
                errorMessage = QString("Unopened '}' bracket at position %1.").arg(i + 1);
                return false;
            }
            inBraces = false;
        }

        // Handle square brackets []
        if (ch == '[') {
            if (inSquareBrackets) {
                errorMessage = QString("Unclosed '[' bracket at position %1.").arg(i + 1);
                return false;
            }
            inSquareBrackets = true;
            temp.clear();
        } else if (ch == ']') {
            if (!inSquareBrackets) {
                errorMessage = QString("Unopened ']' bracket at position %1.").arg(i + 1);
                return false;
            }
            inSquareBrackets = false;

            // Validate bottle name inside []
            if (!bottleNamesList.contains(temp)) {
                errorMessage = QString("Invalid bottle name '%1' at position %2. Must be one of the predefined bottle names.").arg(temp).arg(i + 1 - temp.length());
                return false;
            }
        } else if (inSquareBrackets) {
            temp += ch; // Collect content inside []
        }

        // Handle parentheses ()
        if (ch == '(') {
            if (inParentheses) {
                errorMessage = QString("Unclosed '(' bracket at position %1.").arg(i + 1);
                return false;
            }
            inParentheses = true;
            temp.clear();
        } else if (ch == ')') {
            if (!inParentheses) {
                errorMessage = QString("Unopened ')' bracket at position %1.").arg(i + 1);
                return false;
            }
            inParentheses = false;

            // Validate content inside ()
            if (temp != "S") {
                errorMessage = QString("Content inside parentheses must be 'S' at position %1.").arg(i + 1 - temp.length());
                return false;
            }
        } else if (inParentheses) {
            temp += ch; // Collect content inside ()
        }
    }

    // Check for unclosed brackets
    if (inBraces) {
        errorMessage = "Unclosed '{' bracket.";
        return false;
    }
    if (inSquareBrackets) {
        errorMessage = "Unclosed '[' bracket.";
        return false;
    }
    if (inParentheses) {
        errorMessage = "Unclosed '(' bracket.";
        return false;
    }

    return true; // Input string is valid
}

/* 26-04-2025
 * function kiểm tra toàn bộ sequence nhập từ giếng của người dùng đã hợp lệ hay chưa
 *
 * */
bool amidite::validateAllWells(global_var_t* global_var) {
    QStringList errorMessages; // Lưu trữ tất cả các thông báo lỗi

    for (int wellIndex = 0; wellIndex < MAX_WELL_AMIDITE; ++wellIndex) {
        QString errorMessage;
        bool isValid = validateInput(global_var->amidite_well[wellIndex].string_sequence, bottleNamesList, errorMessage);

        if (!isValid) {
            // Thêm thông báo lỗi kèm số thứ tự giếng
            errorMessages.append(QString("Well %1: %2").arg(wellIndex + 1).arg(errorMessage));
        }
    }

    // Hiển thị kết quả
    if (errorMessages.isEmpty()) {
        QMessageBox::information(nullptr, "Validation Result", "All wells have valid input strings!");
        return true; // Tất cả giếng đều hợp lệ
    } else {
        // Tạo thông báo lỗi tổng hợp
        QString fullErrorMessage = "Errors found in the following wells:\n\n";
        fullErrorMessage += errorMessages.join("\n"); // Kết hợp tất cả lỗi thành một chuỗi

        QMessageBox::warning(nullptr, "Validation Errors", fullErrorMessage);
        return false; // Có ít nhất một giếng không hợp lệ
    }
}
