#include "function.h"
//#define PRINT_DEBUG
#include "filemanager.h"
function::function()
{

}

//============================================ READ STRING AMIDITE LOAD TO TABLE
bool function::read_str_amidite_fromJson(global_var_t* global_var, QString Json_Path)
{
    QString Data_config;
    QFile File_config_Input(Json_Path); // define on macro.h
    if (File_config_Input.open(QIODevice::ReadOnly))
    {
        QTextStream in(&File_config_Input);
        while (!in.atEnd())
        {
            Data_config += in.readLine();
        }
    }
    else
    {
        qDebug()<<"Error open file"<< File_config_Input.errorString();
#ifdef PRINT_DEBUG
        qDebug()<<"Error open file"<< File_config_Input.errorString();
#endif
        return 0;
    }
    File_config_Input.close(); // file close
    QByteArray jsonData_config_input = Data_config.toUtf8();

    //Assign the json text to a JSON object
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData_config_input);
    if(jsonDocument.isObject() == false)
    {
#ifdef PRINT_DEBUG
        qDebug() << "It is not a JSON object";
#endif
    }

    //Then get the main JSON object and get the datas in it
    QJsonObject json_parameter_config = jsonDocument.object();

    QJsonValue QJSON_STRINGWELL [MAX_WELL_AMIDITE];
    QString str_name_obj = "well_";
    QJsonObject valve_para[MAX_WELL_AMIDITE];
    for(int i = 0; i < MAX_WELL_AMIDITE ; i ++)
    {
        QString str2 = str_name_obj +  QString::number(i);
        valve_para[i] = json_parameter_config.value(str2).toObject();

        QJsonValue Name =  valve_para[i].value("Name");
        QJsonValue Sequence =  valve_para[i].value("Sequence");
        global_var->amidite_well[i].string_sequence = Sequence.toString();
        global_var->amidite_well[i].string_name = Name.toString();

#ifdef PRINT_DEBUG
        qDebug()<< "WELL"<< i << " : "<< global_var->amidite_well[i].string_sequence;
        qDebug()<< "Valve_"<< i << " :"<< global_var->amidite_well[i].string_name;
        //        qDebug()<< "Valve_"<< i << " :"<< global_var->valve_setting[i].b;
#endif
    }
    return true;
}


QString function::generateNumberString(int number) {
    QStringList strList;

    for (int i = 0; i < number; ++i) {
        strList.append(QString::number(i));
    }

    return strList.join(",");
}

//============================================================ getData2AmiditeProcess
//void function::getData2AmiditeProcess(global_var_t* global_var)
//{
//    QByteArray  lineData;
//    quint16 u16_length_amidite[MAX_WELL_AMIDITE];
//    for(uint8_t well = 0; well < MAX_WELL_AMIDITE ; well++)
//    {
//        u16_length_amidite[well] = global_var->amidite_well[well].string_sequence.length();

//        std::reverse(global_var->amidite_well[well].string_sequence.begin(), global_var->amidite_well[well].string_sequence.end()); // dong nay reverd data
//        lineData = global_var->amidite_well[well].string_sequence.toLocal8Bit();
//#ifdef PRINT_DEBUG
//        qDebug()<< "STRING well " << lineData;
//        qDebug()<<  " Well : "<<  well<< "length amidite : " << u16_length_amidite[well];
//#endif
//        for(uint8_t u8_sequence = 0; u8_sequence < MAX_SEQUENCE_OF_WELL; u8_sequence++)
//        {
//            global_var->amidite_well[well].u8_sequence[u8_sequence] = char2quint8_Amidite(lineData[u8_sequence]);
//#ifdef PRINT_DEBUG
//            qDebug()<< "Data well " << well;
//            qDebug()<< "w" << well<<" seq" << u8_sequence<< ":"<< global_var->amidite_well[well].u8_sequence[u8_sequence];
//#endif
//        }
//        std::reverse(global_var->amidite_well[well].string_sequence.begin(), global_var->amidite_well[well].string_sequence.end()); // dong nay reverd data
//    }
//    global_var->signal_status_oligo.u16_max_sequence_amidite_setting = u16_length_amidite[0]; // giả sử max là phần tử đầu tiên trong mảng
//    for (int i = 1; i < MAX_WELL_AMIDITE; i++) {
//        if (u16_length_amidite[i] > global_var->signal_status_oligo.u16_max_sequence_amidite_setting) { // nếu phần tử thứ i lớn hơn max, cập nhật max
//            global_var->signal_status_oligo.u16_max_sequence_amidite_setting = u16_length_amidite[i];
//        }
//    }
//#ifdef PRINT_DEBUG
//    //qDebug()<<  " max amidite sequence : "<< global_var->amidite_well->u16_max_sequence_amidite_setting;
//#endif
//}

// ===================================================================== Convert data to amidite SYNO24X
//void function::amiditeSyno24X(global_var_t* global_var)
//{
//    QByteArray  lineData;
//    quint16 u16_length_amidite[MAX_WELL_AMIDITE];
//    for(uint8_t well = 0; well < MAX_WELL_AMIDITE ; well++)
//    {
//        std::reverse(global_var->amidite_well[well].string_sequence.begin(), global_var->amidite_well[well].string_sequence.end()); // dong nay reverd data
//        for(uint8_t u8_sequence = 0; u8_sequence < MAX_SEQUENCE_OF_WELL; u8_sequence++)
//        {
//            global_var->amidite_well[well].u8_sequence[u8_sequence] = char2quint8_Amidite(lineData[u8_sequence]);
//        }
//        std::reverse(global_var->amidite_well[well].string_sequence.begin(), global_var->amidite_well[well].string_sequence.end()); // dong nay reverd data
//    }
//}
// Return value to fill chemical amiite
quint8 function::char2quint8_Amidite(char DataIn)
{
    switch (DataIn)
    {
    /*
    AMD_A = 0,
    AMD_T = 1,
    AMD_G = 2,
    AMD_C = 3,
    AMD_I = 4,
    AMD_U = 5,
    */
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
        /*
    AMD_Y = 6,
    AMD_R = 7,
    AMD_W = 8,
    AMD_S = 9,
    AMD_K = 10,
    AMD_M = 11,
    AMD_D = 12,
    AMD_V = 13,
    AMD_N = 14,
     * */
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
//===============================================================================================================================================================================
void function :: read_protocol_fromJson(protocol_t* p_protocol, QString Json_Path)
{
    QString Data_config;
    QJsonObject JsonObj_step[MAX_STEP_OF_SUB];
    QJsonObject Json_sub_content[MAX_SUB_OF_PROTOCOL];
    QJsonObject Json_PROTOCOL_content;
    QString  str_name_json_step  = "Step_";
    QString  str_name_json_sub = "Sub_";
    QFile File_config_Input(Json_Path); // define on macro.h
    // bat dau doc file
    if (File_config_Input.open(QIODevice::ReadOnly))
    {
        QTextStream in(&File_config_Input);
        while (!in.atEnd())
        {
            Data_config += in.readLine();
        }
    }
    else
    {
#ifdef PRINT_DEBUG
        qDebug()<<"Lỗi Load file Json"<< File_config_Input.errorString();
#endif
    }
    File_config_Input.close(); // file close
    QByteArray jsonData_config_input = Data_config.toUtf8();
    //Assign the json text to a JSON object
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData_config_input);
    if(jsonDocument.isObject() == false)
    {
#ifdef PRINT_DEBUG
        qDebug() << "It is not a JSON object";
#endif
    }
    //Then get the main JSON object and get the datas in it
    Json_PROTOCOL_content = jsonDocument.object();
#ifdef PRINT_DEBUG
    qDebug() << " Read JSON object"<< Json_PROTOCOL_content[0];
#endif
    QJsonValue u8_number_sub = Json_PROTOCOL_content.value("u8_number_sub");
    p_protocol->u8_number_sub = u8_number_sub.toInt();
    QString  str_name_obj ="";
    for(int u8_counter_sub = 0; u8_counter_sub < MAX_SUB_OF_PROTOCOL; u8_counter_sub++)
    {
        str_name_obj = str_name_json_sub + QString::number(u8_counter_sub);
        Json_sub_content[u8_counter_sub] = Json_PROTOCOL_content.value(str_name_obj).toObject();
        QJsonValue u8_number_base_on_sub = Json_sub_content[u8_counter_sub].value("u8_number_base_on_sub");
        QJsonValue u8_number_step_on_base = Json_sub_content[u8_counter_sub].value("u8_number_step_on_base");

        p_protocol->sub[u8_counter_sub].u8_number_base_on_sub = u8_number_base_on_sub.toInt();
        p_protocol->sub[u8_counter_sub].u8_number_step_on_base = u8_number_step_on_base.toInt();
#ifdef PRINT_DEBUG
        qDebug()<< "function u8_number_step_on_base"<<  p_protocol->sub[u8_counter_sub].u8_number_step_on_base;
        qDebug()<< "function u8_number_base_on_sub"<<    p_protocol->sub[u8_counter_sub].u8_number_base_on_sub;
#endif

        for(quint8 u8_counter_step = 0; u8_counter_step < MAX_STEP_OF_SUB; u8_counter_step++)
        {
            // Add to step to page on protocol
            str_name_obj = str_name_json_step +  QString::number(u8_counter_step);
            JsonObj_step[u8_counter_step] = Json_sub_content[u8_counter_sub].value(str_name_obj).toObject();

            QJsonValue u8_type_sulphite = JsonObj_step[u8_counter_step].value("u8_type_sulphite");
            QJsonValue u16tb_wait_after_fill = JsonObj_step[u8_counter_step].value("u16tb_wait_after_fill");
            QJsonValue u16tb_Volume = JsonObj_step[u8_counter_step].value("u16tb_Volume");
            QJsonValue u8_type_chemical_mix_0 = JsonObj_step[u8_counter_step].value("u8_type_chemical_mix_0");
            QJsonValue u8_type_chemical_mix_1 = JsonObj_step[u8_counter_step].value("u8_type_chemical_mix_1");
            QJsonValue u8_type_chemical_mix_2 = JsonObj_step[u8_counter_step].value("u8_type_chemical_mix_2");
            QJsonValue u16tb_Volume_mix_0 = JsonObj_step[u8_counter_step].value("u16tb_Volume_mix_0");
            QJsonValue u16tb_Volume_mix_1= JsonObj_step[u8_counter_step].value("u16tb_Volume_mix_1");
            QJsonValue u16tb_Volume_mix_2 = JsonObj_step[u8_counter_step].value("u16tb_Volume_mix_2");
            QJsonValue douple_coupling_option = JsonObj_step[u8_counter_step].value("douple_coupling_option");

            QJsonValue u8_option_pressure_0 = JsonObj_step[u8_counter_step].value("u8_option_pressure_0");
            QJsonValue u16tb_procs_time_0 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_0");
            QJsonValue u16tb_waitting_after_time_0 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_0");
            QJsonValue u8_option_pressure_1 = JsonObj_step[u8_counter_step].value("u8_option_pressure_1");
            QJsonValue u16tb_procs_time_1 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_1");
            QJsonValue u16tb_waitting_after_time_1 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_1");
            QJsonValue u8_option_pressure_2 = JsonObj_step[u8_counter_step].value("u8_option_pressure_2");
            QJsonValue u16tb_procs_time_2 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_2");
            QJsonValue u16tb_waitting_after_time_2 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_2");
            QJsonValue u8_option_pressure_3 = JsonObj_step[u8_counter_step].value("u8_option_pressure_3");
            QJsonValue u16tb_procs_time_3 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_3");
            QJsonValue u16tb_waitting_after_time_3 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_3");
            QJsonValue u8_option_pressure_4 = JsonObj_step[u8_counter_step].value("u8_option_pressure_4");
            QJsonValue u16tb_procs_time_4 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_4");
            QJsonValue u16tb_waitting_after_time_4 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_4");
            QJsonValue u8_option_pressure_5 = JsonObj_step[u8_counter_step].value("u8_option_pressure_5");
            QJsonValue u16tb_procs_time_5 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_5");
            QJsonValue u16tb_waitting_after_time_5 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_5");
            QJsonValue u8_option_pressure_6 = JsonObj_step[u8_counter_step].value("u8_option_pressure_6");
            QJsonValue u16tb_procs_time_6 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_6");
            QJsonValue u16tb_waitting_after_time_6 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_6");
            QJsonValue u8_option_pressure_7 = JsonObj_step[u8_counter_step].value("u8_option_pressure_7");
            QJsonValue u16tb_procs_time_7 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_7");
            QJsonValue u16tb_waitting_after_time_7 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_7");
            QJsonValue u8_option_pressure_8 = JsonObj_step[u8_counter_step].value("u8_option_pressure_8");
            QJsonValue u16tb_procs_time_8 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_8");
            QJsonValue u16tb_waitting_after_time_8 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_8");
            QJsonValue u8_option_pressure_9 = JsonObj_step[u8_counter_step].value("u8_option_pressure_9");
            QJsonValue u16tb_procs_time_9 = JsonObj_step[u8_counter_step].value("u16tb_procs_time_9");
            QJsonValue u16tb_waitting_after_time_9 = JsonObj_step[u8_counter_step].value("u16tb_waitting_after_time_9");

            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical = u8_type_sulphite.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_Volume.Data = u16tb_Volume.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Data =u16tb_wait_after_fill.toInt();

            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[0] = u8_type_chemical_mix_0.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[1] = u8_type_chemical_mix_1.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[2] = u8_type_chemical_mix_2.toInt();

            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[0].Data = u16tb_Volume_mix_0.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[1].Data = u16tb_Volume_mix_1.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[2].Data = u16tb_Volume_mix_2.toInt();
            p_protocol->sub[u8_counter_sub].douple_coupling_option = douple_coupling_option.toInt();
            //0
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[0] = u8_option_pressure_0.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[0].Data = u16tb_procs_time_0.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[0].Data = u16tb_waitting_after_time_0.toInt();
            //1
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[1] = u8_option_pressure_1.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[1].Data = u16tb_procs_time_1.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[1].Data = u16tb_waitting_after_time_1.toInt();
            //2
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[2] = u8_option_pressure_2.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[2].Data = u16tb_procs_time_2.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[2].Data = u16tb_waitting_after_time_2.toInt();
            //3
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[3] = u8_option_pressure_3.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[3].Data = u16tb_procs_time_3.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[3].Data = u16tb_waitting_after_time_3.toInt();
            //4
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[4] = u8_option_pressure_4.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[4].Data = u16tb_procs_time_4.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[4].Data = u16tb_waitting_after_time_4.toInt();
            //5
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[5] = u8_option_pressure_5.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[5].Data = u16tb_procs_time_5.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[5].Data = u16tb_waitting_after_time_5.toInt();
            //6
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[6] = u8_option_pressure_6.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[6].Data = u16tb_procs_time_6.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[6].Data = u16tb_waitting_after_time_6.toInt();
            //7
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[7] = u8_option_pressure_7.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[7].Data = u16tb_procs_time_7.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[7].Data = u16tb_waitting_after_time_7.toInt();
            //8
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[8] = u8_option_pressure_8.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[8].Data = u16tb_procs_time_8.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[8].Data = u16tb_waitting_after_time_8.toInt();
            //9
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[9] = u8_option_pressure_9.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[9].Data = u16tb_procs_time_9.toInt();
            p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[9].Data = u16tb_waitting_after_time_9.toInt();
        }
    }
}
void function::save_protocol_toJson(protocol_t* p_protocol, QString Json_Path)
{
    //================
    QFile file(Json_Path); // path file
    if (!(file.open(QIODevice::WriteOnly | QFile::Text)))
    {
#ifdef PRINT_DEBUG
        qDebug()<<"Error open file"<< file.errorString();
#endif
        return;
    }
    else
    {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Save setting to File~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QTextStream config_file(&file);
        QJsonObject JsonObj_step[MAX_STEP_OF_SUB];
        QJsonObject Json_sub_content[MAX_SUB_OF_PROTOCOL];
        QJsonObject Json_PROTOCOL_content;
        QString  str_name_json_step  = "Step_";
        QString  str_name_json_sub = "Sub_";
        QString  str_name_obj = "";
        // get setting to QJsonObject
        for(uint8_t u8_counter_sub = 0; u8_counter_sub < MAX_SUB_OF_PROTOCOL; u8_counter_sub++)
        {
            for(uint8_t u8_counter_step = 0; u8_counter_step < MAX_STEP_OF_SUB ; u8_counter_step++)
            {
                JsonObj_step[u8_counter_step].insert("u8_type_sulphite", p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical);
                JsonObj_step[u8_counter_step].insert("u16tb_wait_after_fill", p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Data);
                JsonObj_step[u8_counter_step].insert("u16tb_Volume", p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_Volume.Data);
                JsonObj_step[u8_counter_step].insert("u8_type_chemical_mix_0",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[0]);
                JsonObj_step[u8_counter_step].insert("u8_type_chemical_mix_1",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[1]);
                JsonObj_step[u8_counter_step].insert("u8_type_chemical_mix_2",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[2]);

                JsonObj_step[u8_counter_step].insert("u16tb_Volume_mix_0",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[0].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_Volume_mix_1",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[1].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_Volume_mix_2",p_protocol->sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[2].Data);
                JsonObj_step[u8_counter_step].insert("douple_coupling_option", p_protocol->sub[u8_counter_sub].douple_coupling_option);
                //0
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_0",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[0]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_0",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[0].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_0",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[0].Data);

                //1
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_1",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[1]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_1",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[1].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_1",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[1].Data);
                //2
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_2",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[2]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_2",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[2].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_2",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[2].Data);

                //3
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_3",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[3]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_3",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[3].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_3",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[3].Data);
                //4
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_4",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[4]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_4",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[4].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_4",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[4].Data);

                //5
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_5",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[5]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_5",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[5].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_5",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[5].Data);

                //6
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_6",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[6]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_6",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[6].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_6",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[6].Data);

                //7
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_7",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[7]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_7",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[7].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_7",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[7].Data);
                //8
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_8",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[8]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_8",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[8].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_8",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[8].Data);

                //9
                JsonObj_step[u8_counter_step].insert("u8_option_pressure_9",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[9]);
                JsonObj_step[u8_counter_step].insert("u16tb_procs_time_9",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[9].Data);
                JsonObj_step[u8_counter_step].insert("u16tb_waitting_after_time_9",p_protocol->sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[9].Data);

                // Add to step to page on protocol
                str_name_obj = str_name_json_step +  QString::number(u8_counter_step);
                Json_sub_content[u8_counter_sub].insert( str_name_obj, JsonObj_step[u8_counter_step]);


            } // end for step
            // add 15 step to content
        } // end for sub
        // saveto Json
        str_name_obj ="";
        for(quint8 counter_sub = 0; counter_sub < MAX_SUB_OF_PROTOCOL; counter_sub++)
        {
            str_name_obj = str_name_json_sub + QString::number(counter_sub);
            Json_sub_content[counter_sub].insert("u8_number_base_on_sub",p_protocol->sub[counter_sub].u8_number_base_on_sub);
            Json_sub_content[counter_sub].insert("u8_number_step_on_base",p_protocol->sub[counter_sub].u8_number_step_on_base);
            Json_PROTOCOL_content.insert( str_name_obj, Json_sub_content[counter_sub]);\

        }
        Json_PROTOCOL_content.insert("u8_number_sub",p_protocol->u8_number_sub);
        //        Json_PROTOCOL_content.insert( "Page1", Json_page_content[1]);
        //        Json_PROTOCOL_content.insert( "Page2", Json_page_content[2]);
        QJsonDocument document;
        document.setObject(Json_PROTOCOL_content);

        QByteArray bytes = document.toJson( QJsonDocument::Indented );
        QFile File_Save(Json_Path); // path file
        if( File_Save.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
        {
            QTextStream iStream( &File_Save );
            iStream.setCodec( "utf-8" );
            iStream << bytes;
            File_Save.close();
        }
        else
        {
#ifdef PRINT_DEBUG
            qDebug()<< "false open file";
#endif
        }
    }
}

quint16 function::valve_calculator_timefill(global_var_t* global_var, uint8_t type_sulphite, uint16_t u16_Volume)
{
    if(u16_Volume == 0)
    {
        return 0;
    }
    else
    {
        double db_time = u16_Volume * global_var->valve_setting[type_sulphite].a + global_var->valve_setting[type_sulphite].b;
#ifdef PRINT_DEBUG
        qDebug()<< "function.h ";
        qDebug()<< "type chemical "<<   type_sulphite;
        qDebug()<< "u16_Volume set"<<   u16_Volume;
        qDebug()<< "time Volume "<<  db_time;
#endif
        return (quint16)(db_time);
    }
}

