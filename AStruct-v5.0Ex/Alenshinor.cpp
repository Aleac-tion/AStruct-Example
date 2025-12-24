#include "Alenshinor.h"
#include <cmath>
#include <iostream>
#include <string>
#include <QString>
#include <QMessageBox>
#include <AStruct.h>
#pragma comment(lib,"AStruct.lib")


int Alenshinor::genrator_dmg(
    float attack_multi,    // 攻击区
    float skill_multi,     // 倍率区
    float crit_multi,      // 暴击区
    float dmg_bonus,       // 增伤区
    float reaction_multi,  // 元素反应区
    float resistance_multi,// 抗性区
    float defense_multi    // 防御区
) {
    // 七大乘区直接相乘
    double final_damage = static_cast<double>(attack_multi)
        * static_cast<double>(skill_multi)
        * static_cast<double>(crit_multi)
        * static_cast<double>(dmg_bonus)
        * static_cast<double>(reaction_multi)
        * static_cast<double>(resistance_multi)
        * static_cast<double>(defense_multi);

    return static_cast<int>(std::round(final_damage));
}

Alenshinor::Alenshinor(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    as.loaddata(AStruct::static_getDir() + "/dmg.Astruct");
    

    {
    list << as.getvalue("template", "dmgtable", "atk")
        << as.getvalue("template", "dmgtable", "atks")
        << as.getvalue("template", "dmgtable", "def")
        << as.getvalue("template", "dmgtable", "res")
        << as.getvalue("template", "dmgtable", "up")
        << as.getvalue("template", "dmgtable", "masterup")
        << as.getvalue("template", "dmgtable", "master")
        << as.getvalue("template", "dmgtable", "deathwithlife")
        << as.getvalue("template", "dmgtable", "firework");
    }


    {
        dmg_model = new QStandardItemModel(0, 6);
        ui.dmg_table->setModel(dmg_model);
        ui.dmg_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui.dmg_table->horizontalHeader()->resizeSection(0, 200);
        ui.dmg_table->horizontalHeader()->resizeSection(1, 254); //1270
        ui.dmg_table->horizontalHeader()->resizeSection(2, 254);
        ui.dmg_table->horizontalHeader()->resizeSection(3, 254);
        ui.dmg_table->horizontalHeader()->resizeSection(4, 254);
        ui.dmg_table->horizontalHeader()->resizeSection(5, 54);

        dmg_model->setHorizontalHeaderItem(0, new QStandardItem("伤害(纯)"));
        dmg_model->setHorizontalHeaderItem(1, new QStandardItem("火打冰融化伤害"));
        dmg_model->setHorizontalHeaderItem(2, new QStandardItem("火打水蒸发伤害"));
        dmg_model->setHorizontalHeaderItem(3, new QStandardItem("冰打火融化伤害"));
        dmg_model->setHorizontalHeaderItem(4, new QStandardItem("水打火蒸发伤害"));
        dmg_model->setHorizontalHeaderItem(5, new QStandardItem("对比"));
    }

    {
        up_model=new QStandardItemModel(0, 4);
        ui.up_table->setModel(up_model);
        ui.up_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui.up_table->horizontalHeader()->resizeSection(0, 177.75);
        ui.up_table->horizontalHeader()->resizeSection(1, 177.75);
        ui.up_table->horizontalHeader()->resizeSection(2, 177.75);
        ui.up_table->horizontalHeader()->resizeSection(3, 177.75);
        up_model->setHorizontalHeaderItem(0, new QStandardItem("增幅名"));
        up_model->setHorizontalHeaderItem(1, new QStandardItem("类型"));
        up_model->setHorizontalHeaderItem(2, new QStandardItem("描述"));
        up_model->setHorizontalHeaderItem(3, new QStandardItem("数值"));     
        
    }
      
    {      
        table_model= new QStandardItemModel(0, 1);
        ui.tableview->setModel(table_model);
        ui.tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui.tableview->horizontalHeader()->resizeSection(0, 481);
        table_model->setHorizontalHeaderItem(0, new QStandardItem("增幅名"));
        ui.tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        for (int i = 0; i < list.size(); i++) {
            table_model->setItem(table_model->rowCount(), 0, new QStandardItem(list[i].Go()[0].operator std::string().c_str()));
        }
    }


      
   
}

void Alenshinor::map_dmg() {
    attack = std::stof( as.getvalue("template","gongzi","attack")); //攻击力
    skill_multi = std::stof(as.getvalue("template", "gongzi", "skill_multi"));   //倍率  
    crit_dmg = std::stof(as.getvalue("template", "gongzi", "crit_dmg"));     // 暴伤
    dmg_bonus = std::stof(as.getvalue("template", "gongzi", "dmg_bonus"));   // 增伤
    mastery_bonus = std::stof(as.getvalue("template", "gongzi", "mastery_bonus"));  //元素反应区域
    resistance_multi = std::stof(as.getvalue("template", "gongzi", "resistance_multi"));   //target_will_dmg; // 抗性区   
    defense_multi = std::stof(as.getvalue("template", "gongzi", "defense_multi"));
}

float Alenshinor::g_res(int c_level, int t_level) {
    float tempa = (level_character + 100) + (level_target + 100);
    float tempb = level_character + 100;
    return (tempb / tempa);
}

float Alenshinor::g_resmulti() {
    std::ostringstream oss;
    float base_anti = ui.lineEdit_7->text().isEmpty() ? 15.0f : ui.lineEdit_7->text().toFloat();
    oss << "基础抗性:" << base_anti;
    float total_anti = 0; //减弱的抗性

    for (int i = 0; i < up_model->rowCount(); i++) {
        QModelIndex typeIndex = up_model->index(i, 0);
        QModelIndex valueIndex = up_model->index(i, 3);

        QString effectType = up_model->data(typeIndex, Qt::DisplayRole).toString();
        QVariant effectValue = up_model->data(valueIndex, Qt::DisplayRole);

        if (effectType == "属性抗性") {
            float percent = effectValue.toFloat();
            total_anti += effectValue.toFloat();
        }
       
    }

    oss << "将要减少的抗性:" << total_anti;
    float resistance = (base_anti + total_anti) / 100.0F;
    oss << "减少后的抗性:" << resistance;
    float target_will_dmg;
    if (resistance < 0) {
        target_will_dmg = 1.0f - (resistance / 2.0f);
    }
    else if (resistance <= 0.75f) {
        target_will_dmg = 1.0f - resistance;
    }
    else {
        // 抗性 > 75% 时：1 / (1 + 4*(抗性 - 0.75))
        target_will_dmg = 1.0f / (1.0f + 4.0f * (resistance - 0.75f));
    }
    return target_will_dmg;
}

void Alenshinor::on_coumpute_clicked() {

    level_character = ui.lineEdit->text().toFloat();//角色等级
    level_target = ui.lineEdit_2->text().toFloat(); //怪物等级
    

    float def = g_res(level_character, level_target);//目标防御力
    float target_res = 0.25f;//目标抗性区//0.75
    float attack_fate = ui.lineEdit_5->text().toFloat() / 100.0F;
    float crit_dmg_pre = ui.lineEdit_6->text().toFloat() / 100.0F;    
    get_atk();
    get_bonus();
    attack = ui.lineEdit_9->text().toFloat();
    skill_multi = attack_fate;
    crit_dmg = crit_dmg_pre+1.0F;
    dmg_bonus = ui.lineEdit_11->text().toFloat() / 100.0F;  
    resistance_multi = g_resmulti();
    defense_multi = def;
    std::ostringstream oss;
    oss << "攻击区:" << attack << " 倍率区:" << skill_multi << " 暴击区:" << crit_dmg << " 增伤区:" << dmg_bonus << " 公子反应:" << get_mas_bonus(2.0F)
        << " 抗性区:" << resistance_multi << " 防御区:" << defense_multi;
    //反应模型
    {
        int damage = genrator_dmg(
            attack,       //    攻击区
            skill_multi,  //    倍率区
            crit_dmg,     //    暴击区
            dmg_bonus,    //    增伤区 
            get_mas_bonus(1.0F),//    元素反应区
            resistance_multi,// 抗性区
            defense_multi    // 防御区
        );

        int Big_damage = genrator_dmg(
            attack,       //    攻击区
            skill_multi,  //    倍率区
            crit_dmg,     //    暴击区
            dmg_bonus,    //    增伤区 
            get_mas_bonus(2.0F),//    元素反应区
            resistance_multi,// 抗性区
            defense_multi    // 防御区
        );

        int SMALL_damage = genrator_dmg(
            attack,       //    攻击区
            skill_multi,  //    倍率区
            crit_dmg,     //    暴击区
            dmg_bonus,    //    增伤区 
            get_mas_bonus(1.5F),//    元素反应区
            resistance_multi,// 抗性区
            defense_multi    // 防御区
        );
        dmg_model->setItem(dmg_model->rowCount(), 1, new QStandardItem(QString::number(Big_damage)));
        dmg_model->setItem(dmg_model->rowCount() - 1, 0, new QStandardItem(QString::number(damage)));     
        dmg_model->setItem(dmg_model->rowCount()-1, 4, new QStandardItem(QString::number(Big_damage)));
        dmg_model->setItem(dmg_model->rowCount()-1, 2, new QStandardItem(QString::number(SMALL_damage)));
        dmg_model->setItem(dmg_model->rowCount()-1, 3, new QStandardItem(QString::number(SMALL_damage)));
    
    }
    
    QMessageBox::information(this, "n", oss.str().c_str());
}

void Alenshinor::on_add_clicked() {
    QModelIndex curr_index = ui.tableview->currentIndex();
    int Cur_index= curr_index.row();
    if (Cur_index==-1)return;
    QModelIndex data_table = table_model->index(Cur_index, 0);
    QVariant cur_data = table_model->data(data_table, Qt::DisplayRole);
    up_model->setItem(up_model->rowCount(), 0, new QStandardItem(cur_data.toString()));
    for (int i = 0; i < list.size(); i++) {
        if (searchtext(list[i], cur_data.toString().toStdString())) {
            up_model->setItem(up_model->rowCount()-1, 1, new QStandardItem(list[i].Go()[1].operator std::string().c_str()));
            up_model->setItem(up_model->rowCount()-1, 2, new QStandardItem(list[i].Go()[2].operator std::string().c_str()));
            //up_model->setItem(up_model->rowCount()-1, 3, new QStandardItem(list[i].Go()[3].operator std::string().c_str()));
            return;
        }
    }
}

void Alenshinor::on_del_clicked() {
    QModelIndex curr_index = ui.up_table->currentIndex();
    up_model->removeRow(curr_index.row());
}

void Alenshinor::get_atk() {
    float atk = ui.lineEdit_4->text().isEmpty() ? 1.0f : ui.lineEdit_4->text().toFloat();
    float base_atk = ui.lineEdit_3->text().isEmpty() ? 1.0f : ui.lineEdit_3->text().toFloat();
    

    for (int i = 0; i < up_model->rowCount(); i++) {
        QModelIndex typeIndex = up_model->index(i, 0);
        QModelIndex valueIndex = up_model->index(i, 3);

        QString effectType = up_model->data(typeIndex, Qt::DisplayRole).toString();
        QVariant effectValue = up_model->data(valueIndex, Qt::DisplayRole);

        if (effectType == "攻击力增幅") {
            atk += effectValue.toFloat();
        }
        else if (effectType == "攻击力百分比增幅") {
            float percent = effectValue.toFloat() / 100.0f; // 百分比转小数
            atk += base_atk * percent;
        }
    }

    ui.lineEdit_9->setText(QString::number(atk));//设置最终伤害
}

void Alenshinor::get_bonus() {
    float base_bonus = ui.lineEdit_10->text().isEmpty() ? 0.0f : ui.lineEdit_10->text().toFloat() / 100.0f;
    float bones = 1.0f + base_bonus;  // 初始增伤 = 1 + 基础增伤

    // 2. 遍历up_model中的额外增伤项（如28.8%、25%等）
    for (int i = 0; i < up_model->rowCount(); ++i) {
        QModelIndex typeIndex = up_model->index(i, 0);
        QModelIndex valueIndex = up_model->index(i, 3);

        QString effectType = up_model->data(typeIndex, Qt::DisplayRole).toString();
        if (effectType != "伤害提升") continue;

        // 3. 额外增伤项加算（如28.8% → 0.288）
        float percent = up_model->data(valueIndex).toFloat() / 100.0f;
        bones += percent;  // 核心：所有增伤项【加算】
    }

    // 4. 显示结果（保留4位小数，与目标3.4816完全一致）
    ui.lineEdit_11->setText(QString::number(bones*100.0f));
}

float Alenshinor::get_mas_bonus(float fate) {
    
    float base_master = ui.lineEdit_8->text().isEmpty() ? 0.0f : ui.lineEdit_8->text().toFloat();
    float master_bone = 0.0f;

    for (int i = 0; i < up_model->rowCount(); i++) {
        QModelIndex typeIndex = up_model->index(i, 0);
        QModelIndex valueIndex = up_model->index(i, 3);

        QString effectType = up_model->data(typeIndex, Qt::DisplayRole).toString();
        QVariant effectValue = up_model->data(valueIndex, Qt::DisplayRole);

        if (effectType == "元素精通") {
            base_master += effectValue.toFloat();
        }
        else if (effectType == "反应伤害提升") {           
            float percent = effectValue.toFloat() / 100.0f; // 百分比转小数
            master_bone += percent;
        }
    }

    float tp_a = base_master * 2.78f;
    float tp_b = base_master +1400.0f;
    float master_up = tp_a / tp_b;//元素精通提升
    float bones = 1 + master_up + master_bone;
    float finalmaster = fate * bones;
    ui.lineEdit_12->setText(QString::number(base_master));
    return finalmaster;
}

Alenshinor::~Alenshinor()
{}

