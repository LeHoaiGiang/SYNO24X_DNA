#include "volume_manager.h"
#include "qstring.h"
#include <QStringList>
//#define PRINT_DEBUG
QStringList VALVE_NAME = {"A", "T", "G", "C","a", "t", "g", "c", "F1","F2", "ACTIVATOR", "TCA", "WASH", "OX1", "OX2", "CAP-B", "CAP-A", "AMIDITE"};
volume_manager::volume_manager()
{

}

void volume_manager:: setPath(QString Path)
{
    valveSetting_Path = Path;
}
void volume_manager::add_volume(int16_t type_chemical, uint16_t volume)
{
    if(type_chemical <= CAPPING_CAPA && type_chemical >= 0)
    {
        valve[type_chemical].volume_curent  = valve[type_chemical].volume_curent + volume;
        if(valve[type_chemical].volume_curent > 500000)
        {
            valve[type_chemical].volume_curent = 500000;
        }
        else
        {
            if(valve[type_chemical].volume_curent < 0)
            {
                valve[type_chemical].volume_curent = 0;
            }
        }
    }
    ReloadUIVolumeMNG();
    tableView_display_data();
    /*
    switch(type_chemical)
    {
    case A:
    {
        break;
    }
    case T:
    {
        break;
    }
    case G:
    {
        break;
    }
    case C:
    {
        break;
    }
    case I:
    {
        break;
    }
    case U:
    {
        break;
    }
    case Activator:
    {
        break;
    }
    case TCA_in_DCM:
    {
        break;
    }
    case WASH_ACN_DCM:
    {
        break;
    }
    case OXIDATION_IODINE:
    {
        break;
    }
    case CAPPING_CAPA:
    {
        break;
    }
    case CAPPING_CAPB:
    {
        break;
    }
    default:
    {
        break;
    }
    }
    */
}
void volume_manager::sub_volume(int16_t type_chemical, uint16_t volume)
{
    if(type_chemical <= CAPPING_CAPA && type_chemical >= 0)
    {
        valve[type_chemical].volume_curent  = valve[type_chemical].volume_curent - volume;
        if(valve[type_chemical].volume_curent < 0)
        {
            valve[type_chemical].volume_curent = 0;
        }
    }
    ReloadUIVolumeMNG();
    tableView_display_data();
    /*switch(type_chemical)
    {
    case A:
    {
        break;
    }
    case T:
    {
        break;
    }
    case G:
    {
        break;
    }
    case C:
    {
        break;
    }
    case I:
    {
        break;
    }
    case U:
    {
        break;
    }
    case Activator:
    {
        break;
    }
    case TCA_in_DCM:
    {
        break;
    }
    case WASH_ACN_DCM:
    {
        break;
    }
    case OXIDATION_IODINE:
    {
        break;
    }
    case CAPPING_CAPA:
    {
        break;
    }
    case CAPPING_CAPB:
    {
        break;
    }
    default:
    {
        break;
    }
    }*/
}

void volume_manager::sub_volume_running(uint8_t type_chemical, uint16_t volume)
{
    valve[type_chemical].volume_curent  = valve[type_chemical].volume_curent - volume;
}
void volume_manager::sub_volume_amidite(uint8_t AMIDITE,  uint16_t volume)
{
    uint16_t volume_avg = 0;
    switch (AMIDITE)
    {
    case AMD_A:
    {
        sub_volume(A, volume);
        break;
    }
    case AMD_T:
    {
        sub_volume(T, volume);
        break;
    }
    case AMD_G:
    {
        sub_volume(G, volume);
        break;
    }
    case AMD_C:
    {
        sub_volume(C, volume);
        break;
    }
    case AMD_I:
    {
        sub_volume(I, volume);
        break;
    }
    case AMD_U:
    {
        sub_volume(U, volume);
        break;
    }
    case AMD_Y: // chia 2
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_Y][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_Y][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_R: // div 2
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_R][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_R][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_W:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_W][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_W][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_S:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_S][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_S][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_K:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_K][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_K][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_M:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_M][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_M][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_D:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_Y][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_Y][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_V:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_V][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_V][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_N: // CHIA 4
    {
        volume_avg = volume / 4;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_N][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_N][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_H:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_H][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_H][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_B:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_B][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                sub_volume(MIXED_BASE[AMD_B][idx], volume_avg);
            }
        }
        break;
    }
    }
}
void volume_manager::reset_volume_cal()
{
    for(uint8_t idx_valve = 0; idx_valve < MAX_NUMBER_VALVE ; idx_valve++)
    {
        valve[idx_valve].volume_calculator_need = 0;
    }
}

void  volume_manager::cal_remain_volume()
{
    for(uint8_t idx_valve = 0; idx_valve < MAX_NUMBER_VALVE ; idx_valve++)
    {
        valve[idx_valve].volume_remain = valve[idx_valve].volume_curent - valve[idx_valve].volume_calculator_need;
    }
}
void volume_manager::add_volume_normal_cal(int16_t type_chemical, uint16_t volume)
{
    valve[type_chemical].volume_calculator_need  = valve[type_chemical].volume_calculator_need + volume;
#ifdef PRINT_DEBUG
    qDebug()<< "add_volume_normal_cal" << type_chemical<< " " <<  valve[type_chemical].volume_calculator_need;
#endif
}

void volume_manager::add_volume_amidite_cal(uint8_t AMIDITE,  uint16_t volume)
{
    uint16_t volume_avg = 0;
    switch (AMIDITE)
    {
    case AMD_A:
    {
        add_volume_normal_cal(A, volume);
        break;
    }
    case AMD_T:
    {
        add_volume_normal_cal(T, volume);
        break;
    }
    case AMD_G:
    {
        add_volume_normal_cal(G, volume);
        break;
    }
    case AMD_C:
    {
        add_volume_normal_cal(C, volume);
        break;
    }
    case AMD_a:
    {
        add_volume_normal_cal(a, volume);
        break;
    }
    case AMD_t:
    {
        add_volume_normal_cal(t, volume);
        break;
    }
    case AMD_g:
    {
        add_volume_normal_cal(g, volume);
        break;
    }
    case AMD_c:
    {
        add_volume_normal_cal(c, volume);
        break;
    }
    case AMD_I:
    {
        add_volume_normal_cal(I, volume);
        break;
    }
    case AMD_U:
    {
        add_volume_normal_cal(U, volume);
        break;
    }
    case AMD_Y: // chia 2
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_Y][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_Y][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_R: // div 2
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_R][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_R][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_W:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_W][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_W][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_S:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_S][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_S][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_K:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_K][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_K][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_M:
    {
        volume_avg = volume / 2;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_M][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_M][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_D:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_Y][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_Y][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_V:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_V][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_V][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_N: // CHIA 4
    {
        volume_avg = volume / 4;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_N][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_N][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_H:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_H][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_H][idx], volume_avg);
            }
        }
        break;
    }
    case AMD_B:
    {
        volume_avg = volume / 3;
        if(volume_avg < 10) volume_avg = 10;
        for(uint16_t idx =0; idx < 4 ; idx++)
        {
            if(MIXED_BASE[AMD_B][idx] != CHEMICAL_SUBTANCE_EMPTY)
            {
                add_volume_normal_cal(MIXED_BASE[AMD_B][idx], volume_avg);
            }
        }
        break;
    }
    }
}
void volume_manager::Calculator_Protocol()
{

}
void volume_manager::ReloadUIVolumeMNG()
{
    spbxs_current[A]->setValue(valve[A].volume_curent);
    spbxs_current[T]->setValue(valve[T].volume_curent);
    spbxs_current[G]->setValue(valve[G].volume_curent);
    spbxs_current[C]->setValue(valve[C].volume_curent);
    spbxs_current[I]->setValue(valve[I].volume_curent);
    spbxs_current[U]->setValue(valve[U].volume_curent);
    spbxs_current[a]->setValue(valve[a].volume_curent);
    spbxs_current[t]->setValue(valve[t].volume_curent);
    spbxs_current[g]->setValue(valve[g].volume_curent);
    spbxs_current[c]->setValue(valve[c].volume_curent);

    spbxs_current[Activator]->setValue(valve[Activator].volume_curent);
    spbxs_current[TCA_in_DCM]->setValue(valve[TCA_in_DCM].volume_curent);
    spbxs_current[WASH_ACN_DCM]->setValue(valve[WASH_ACN_DCM].volume_curent);
    spbxs_current[OXIDATION_IODINE]->setValue(valve[OXIDATION_IODINE].volume_curent);
    spbxs_current[CAPPING_CAPA]->setValue(valve[CAPPING_CAPA].volume_curent);
    spbxs_current[CAPPING_CAPB]->setValue(valve[CAPPING_CAPB].volume_curent);
}

void volume_manager:: save_parameter_valve()
{
    // Open file
    QFile file(valveSetting_Path); // path file
    if (!(file.open(QIODevice::WriteOnly)))
    {
#ifdef PRINT_DEBUG
        qDebug()<<"Error open file"<< file.errorString();
#endif
        return;
    }
    else
    {
        QTextStream config_file(&file);
        QJsonObject json_paramterValve_config[MAX_NUMBER_VALVE];
        // Valve
        for(int element = 0; element < MAX_NUMBER_VALVE ; element++)
        {
            json_paramterValve_config[element].insert("a", valve[element].a );
            json_paramterValve_config[element].insert("b", valve[element].b );
            json_paramterValve_config[element].insert("volume_curent", valve[element].volume_curent);
            //json_paramterValve_config[element].insert("b", valve[element].b );
        }
        QJsonObject content;
        QString  str2  = "Valve_";
        for(int i = 0; i < MAX_NUMBER_VALVE ; i++)
        {
            QString  str_name_obj = str2 +  QString::number(i);
            content.insert( str_name_obj, json_paramterValve_config[i]);
        }
        QJsonDocument document;
        document.setObject( content );
        QByteArray bytes = document.toJson( QJsonDocument::Indented );
        QFile File_Save(valveSetting_Path); // path file
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
            qDebug()<< "false";
#endif
        }
    }
}

void volume_manager:: read_parameter_valve()
{
    QString Data_Read;
    QFile File_ref_memorize(valveSetting_Path);
    File_ref_memorize.open(QIODevice::ReadOnly);
    Data_Read = File_ref_memorize.readAll();
    File_ref_memorize.close();
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QJsonDocument json_document_from_DataRead = QJsonDocument::fromJson(Data_Read.toUtf8());
    QJsonObject Js_Obj_from_Document = json_document_from_DataRead.object();
    QString str_name_obj = "Valve_";
    QJsonObject valve_para[MAX_NUMBER_VALVE];
#ifdef PRINT_DEBUG
    qDebug()<< "volume_manager";
#endif
    QStringList faultyValveNames; // Sử dụng QStringList để lưu tên các van bị lỗi
    // VALVE_NAME
    for(int i = 0; i < MAX_NUMBER_VALVE ; i ++)
    {
        QString str2 = str_name_obj +  QString::number(i);
        valve_para[i] = Js_Obj_from_Document.value(str2).toObject();

        QJsonValue a =  valve_para[i].value("a");
        QJsonValue b =  valve_para[i].value("b");
        QJsonValue volume_curent = valve_para[i].value("volume_curent");
        valve[i].a = a.toDouble();
        valve[i].b = b.toDouble();
        valve[i].volume_curent = volume_curent.toInt();

        // Kiểm tra lỗi và thêm tên van vào danh sách
        if (valve[i].a == 0 && valve[i].b == 0)
        {
            QString errorValveName = "Valve : " + VALVE_NAME[i];
            faultyValveNames.append(errorValveName);
        }
#ifdef PRINT_DEBUG
        qDebug()<< "Valve_"<< i << " a:"<< valve[i].a<<" b:"<< valve[i].b << "volume_curent "<< i << " :"<< valve[i].volume_curent;
#endif
    }
    // Hiển thị hộp thoại thông báo duy nhất nếu có van bị lỗi
     if (!faultyValveNames.isEmpty())
     {
         QString message = "Calib again Valve:\n";
         for (const QString& valveName : faultyValveNames)
         {
             message += valveName + "\n";
         }
         QMessageBox::warning(nullptr, "Error Calibration Valve", message);
     }
    //ReloadUIVolumeMNG();
}
void volume_manager::onButtonReleased_Add_Chemical(ORDINAL_VALVE Chemical)
{
    valve[Chemical].volume_curent = valve[Chemical].volume_curent + spbxs_change[Chemical]->value();
    if(valve[Chemical].volume_curent > 500000)
    {
        valve[Chemical].volume_curent = 500000;
    }
    else
    {
        if(valve[Chemical].volume_curent < 0)
        {
            valve[Chemical].volume_curent = 0;
        }
    }
    tableView_display_data();
    spbxs_current[Chemical]->setValue(valve[Chemical].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_Chemical" << Chemical;
#endif
}
void volume_manager::onButtonReleased_Sub_Chemical(ORDINAL_VALVE Chemical)
{
    valve[Chemical].volume_curent = valve[Chemical].volume_curent - spbxs_change[Chemical]->value();
    spbxs_current[Chemical]->setValue(valve[Chemical].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Sub_Chemical" << Chemical<< " " <<  valve[Chemical].volume_curent;
#endif
}
/*
 *
 * comment by Le Hoai Giang == chuyen sang sử dụng mảng để chứa các spbxs giúp cho xử lý nhanh và truy xuất dễ dàng hơn
 *
 *
void volume_manager::onButtonReleased_Add_A()
{
    valve[A].volume_curent = valve[A].volume_curent + spbxs_current[A]->value();
    //spbx_current_vl_A->setValue(valve[A].volume_curent);
    spbxs_current[A]->setValue(valve[A].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_A";
#endif
}
void volume_manager::onButtonReleased_Add_T()
{
    valve[T].volume_curent = valve[T].volume_curent + spbxs_current[T]->value();
    //spbx_current_vl_T->setValue(valve[T].volume_curent);
    spbxs_current[T]->setValue(valve[T].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_T";
#endif
}
void volume_manager::onButtonReleased_Add_G()
{
    valve[G].volume_curent = valve[G].volume_curent + spbxs_current[G]->value();
    spbxs_current[G]->setValue(valve[G].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_T";
#endif
}
void volume_manager::onButtonReleased_Add_C()
{
    valve[C].volume_curent = valve[C].volume_curent + spbxs_current[C]->value();
    spbxs_current[C]->setValue(valve[C].volume_curent);

#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_C";
#endif
}
void volume_manager::onButtonReleased_Add_F1()
{
    valve[I].volume_curent = valve[I].volume_curent + spbx_change_vl_F1->value();
    spbx_current_vl_F1->setValue(valve[I].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F1";
#endif
}
void volume_manager::onButtonReleased_Add_F2()
{
    valve[U].volume_curent = valve[U].volume_curent + spbx_change_vl_F2->value();
    spbx_current_vl_F2->setValue(valve[U].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F2";
#endif
}
void volume_manager::onButtonReleased_Add_ACT()
{
    valve[U].volume_curent = valve[U].volume_curent + spbx_change_vl_ACT->value();
    spbx_current_vl_ACT->setValue(valve[U].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F2";
#endif
}
void volume_manager::onButtonReleased_Add_TCA()
{
    valve[TCA_in_DCM].volume_curent = valve[TCA_in_DCM].volume_curent + spbx_change_vl_TCA->value();
    spbx_current_vl_TCA->setValue(valve[TCA_in_DCM].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F2";
#endif
}
void volume_manager::onButtonReleased_Add_WASH()
{
    valve[WASH_ACN_DCM].volume_curent = valve[WASH_ACN_DCM].volume_curent + spbx_change_vl_WASH->value();
    spbx_current_vl_WASH->setValue(valve[WASH_ACN_DCM].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F2";
#endif
}
void volume_manager::onButtonReleased_Add_OX()
{
    valve[OXIDATION_IODINE].volume_curent = valve[OXIDATION_IODINE].volume_curent + spbx_change_vl_OX->value();
    spbx_current_vl_OX->setValue(valve[OXIDATION_IODINE].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_F2";
#endif
}
void volume_manager::onButtonReleased_Add_CAPA()
{
    valve[CAPPING_CAPA].volume_curent = valve[CAPPING_CAPA].volume_curent + spbx_change_vl_CAPA->value();
    spbx_current_vl_CAPA->setValue(valve[CAPPING_CAPA].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_CAPA";
#endif
}
void volume_manager::onButtonReleased_Add_CAPB()
{
    valve[CAPPING_CAPB].volume_curent = valve[CAPPING_CAPB].volume_curent + spbx_change_vl_CAPB->value();
    spbx_current_vl_CAPB->setValue(valve[CAPPING_CAPB].volume_curent);
#ifdef PRINT_DEBUG
    qDebug()<< "onButtonReleased_Add_CAPB";
#endif
}
*/

/*
 * LeHoaiGiang
 * 17-02-2024
 * tableView_init
 * funtion set stylesheet to tableview
*/
void volume_manager::tableView_init()
{
    QStringList horizontalHeader = QStringList() << "Name" << "Current Volume"<< "Protocol Use"<<"Volume After Run";
    model->setHorizontalHeaderLabels(horizontalHeader);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//
    tableView->setModel(model);
    tableView_display_data();
}
/*
 * LeHoaiGiang
 * 17-02-2024
 * tableView_init
 * funtion display data to table
*/
void volume_manager::tableView_display_data()
{
    int32_t temp_Data =0;
    // Populate the model with data
    // test fill data to table
    //    for (int rowIndex = 0; rowIndex < MAX_VALVE; ++rowIndex) {
    //        for (int columnIndex = 0; columnIndex < 3; ++columnIndex) {
    //            QStandardItem *item = new QStandardItem(QString(QString::number(rowIndex)));
    //            model->setItem(rowIndex, columnIndex, item);
    //        }
    //    }
    for (int rowIndex = 0; rowIndex < MAX_VALVE; ++rowIndex)
    {

        QModelIndex index = model->index(rowIndex, NAME_CHEMICAL_IDX , QModelIndex());
        model->setData(index,NAME_CHEMICAL[rowIndex]);
    }
    for (int rowIndex = 0; rowIndex < MAX_VALVE; ++rowIndex)
    {

        QModelIndex index = model->index(rowIndex, CURRENT_VOLUME_IDX , QModelIndex());
        model->setData(index,valve[rowIndex].volume_curent);
    }
    for (int rowIndex = 0; rowIndex < MAX_VALVE; ++rowIndex)
    {
        QModelIndex index = model->index(rowIndex, CALCULATOR_VOLUME_IDX , QModelIndex());
        model->setData(index,valve[rowIndex].volume_calculator_need);
    }
    // KIỂM TRA HOÁ CHẤT CÓ BỊ THIẾU CHO VIỆC RUN KHÔNG
    for (int rowIndex = 0; rowIndex < MAX_VALVE; ++rowIndex)
    {
        QModelIndex index = model->index(rowIndex, REMAIN_VOLUME_IDX , QModelIndex());
        model->setData(index,valve[rowIndex].volume_remain);
        if(valve[rowIndex].volume_remain < 0)
        {
            setCellColor( rowIndex, REMAIN_VOLUME_IDX, Qt::red); // set màu cho ô bị thiếu hoá chất
        }
        else
        {
            setCellColor( rowIndex, REMAIN_VOLUME_IDX, Qt::white); // set màu cho ô bị thiếu hoá chất
        }
    }
}

/*
 * LeHoaiGiang
 * 17-02-2024
 * tableView_init
 * funtion set color to tableview for index table not enough volume for protocol
*/
void volume_manager::setCellColor(int rowIndex, int columnIndex, QColor color)
{
    // Lấy model của bảng
    QStandardItemModel *model = static_cast<QStandardItemModel*>(tableView->model());
    // Kiểm tra rowIndex và columnIndex hợp lệ
    if (rowIndex < 0 || rowIndex >= model->rowCount() || columnIndex < 0 || columnIndex >= model->columnCount()) {
        return;
    }
    // Tạo QBrush với màu đỏ
    QBrush brush(color);
    // Thiết lập brush cho item tại vị trí rowIndex, columnIndex
    model->setData(model->index(rowIndex, columnIndex), brush, Qt::BackgroundRole);
}


quint16 volume_manager::valve_calculator_timefill( uint8_t type_sulphite, uint16_t u16_Volume)
{
    if(u16_Volume == 0)
    {
        return 0;
    }
    else
    {
        double db_time = u16_Volume * valve[type_sulphite].a + valve[type_sulphite].b;
#ifdef PRINT_DEBUG
        qDebug()<< "function.h ";
        qDebug()<< "type chemical "<<   type_sulphite;
        qDebug()<< "u16_Volume set"<<   u16_Volume;
        qDebug()<< "time Volume "<<  db_time;
#endif
        return (quint16)(db_time);
    }
}
