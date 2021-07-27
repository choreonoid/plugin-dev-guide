#include "BodyPositionItem.h"
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ItemFileIO>
#include <cnoid/ValueTree>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>

using namespace std;
using namespace cnoid;

namespace {

class BodyPositionItemCreationPanel : public ItemCreationPanelBase<BodyPositionItem>
{
    QLineEdit* nameEntry;
    QDoubleSpinBox* heightSpin;
    QComboBox* colorCombo;
    
public:
    BodyPositionItemCreationPanel()
    {
        auto vbox = new QVBoxLayout;
        setLayout(vbox);

        auto hbox1 = new QHBoxLayout;
        hbox1->addWidget(new QLabel("Name :"));
        nameEntry = new QLineEdit;
        hbox1->addWidget(nameEntry);
        vbox->addLayout(hbox1);

        auto hbox2 = new QHBoxLayout;

        hbox2->addWidget(new QLabel("Flag height :"));
        heightSpin = new QDoubleSpinBox;
        heightSpin->setRange(0.01, 9.99);
        heightSpin->setDecimals(2);
        heightSpin->setValue(1.0);
        hbox2->addWidget(heightSpin);

        hbox2->addWidget(new QLabel("Color :"));
        colorCombo = new QComboBox;
        colorCombo->addItem("Red");
        colorCombo->addItem("Green");
        colorCombo->addItem("Blue");
        hbox2->addWidget(colorCombo);

        vbox->addLayout(hbox2);
    }

    virtual bool initializeCreation(BodyPositionItem* protoItem, Item* parentItem) override
    {
        nameEntry->setText(protoItem->name().c_str());
        return true;
    }
    
    virtual bool updateItem(BodyPositionItem* protoItem, Item* parentItem) override
    {
        protoItem->setName(nameEntry->text().toStdString());
        protoItem->setFlagHeight(heightSpin->value());
        protoItem->setFlagColor(colorCombo->currentIndex());

        if(auto bodyItem = dynamic_cast<BodyItem*>(parentItem)){
            protoItem->setPosition(bodyItem->body()->rootLink()->position());
        }
        
        return true;
    }
};

class BodyPositionItemFileIO : public ItemFileIoBase<BodyPositionItem>
{
    BodyPositionItem::LengthUnit lengthUnit;
    BodyPositionItem::AngleUnit angleUnit;

    QWidget* panel;
    QComboBox* lengthUnitCombo;
    QComboBox* angleUnitCombo;
    
public:
    BodyPositionItemFileIO()
        : ItemFileIoBase<BodyPositionItem>(
            "BODY-POSITION",
            Load | Save | Options | OptionPanelForLoading | OptionPanelForSaving)
    {
        setCaption("Body Position");
        setExtension("pos");
        resetOptions();
        panel = nullptr;
    }

    virtual void resetOptions() override
    {
        lengthUnit = BodyPositionItem::Meter;
        angleUnit = BodyPositionItem::Degree;
    }

    virtual void storeOptions(Mapping* options) override
    {
        if(lengthUnit == BodyPositionItem::Millimeter){
            options->write("length_unit", "millimeter");
        } else {
            options->write("length_unit", "meter");
        }
        if(angleUnit == BodyPositionItem::Radian){
            options->write("angle_unit", "radian");
        } else {
            options->write("angle_unit", "degree");
        }
    }

    virtual bool restoreOptions(const Mapping* options) override
    {
        string unit;
        options->read("length_unit", unit);
        if(unit == "millimeter"){
            lengthUnit = BodyPositionItem::Millimeter;
        } else {
            lengthUnit = BodyPositionItem::Meter;
        }
        options->read("angle_unit", unit);
        if(unit == "radian"){
            angleUnit = BodyPositionItem::Radian;
        } else {
            angleUnit = BodyPositionItem::Degree;
        }
        return true;
    }

    QWidget* getOrCreateOptionPanel()
    {
        if(!panel){
            panel = new QWidget;
            auto hbox = new QHBoxLayout;
            panel->setLayout(hbox);

            hbox->addWidget(new QLabel("Length unit"));
            lengthUnitCombo = new QComboBox;
            lengthUnitCombo->addItem("Meter");
            lengthUnitCombo->addItem("Millimeter");
            hbox->addWidget(lengthUnitCombo);

            hbox->addWidget(new QLabel("Angle unit"));
            angleUnitCombo = new QComboBox;
            angleUnitCombo->addItem("Degree");
            angleUnitCombo->addItem("Radian");
            hbox->addWidget(angleUnitCombo);
        }
        return panel;
    }

    void fetchOptionPanel()
    {
        if(lengthUnitCombo->currentIndex() == 0){
            lengthUnit = BodyPositionItem::Meter;
        } else {
            lengthUnit = BodyPositionItem::Millimeter;
        }
        if(angleUnitCombo->currentIndex() == 0){
            angleUnit = BodyPositionItem::Degree;
        } else {
            angleUnit = BodyPositionItem::Radian;
        }
    }

    virtual QWidget* getOptionPanelForLoading() override
    {
        return getOrCreateOptionPanel();
    }

    virtual void fetchOptionPanelForLoading() override
    {
        fetchOptionPanel();
    }

    virtual bool load(BodyPositionItem* item, const std::string& filename) override
    {
        return item->loadBodyPosition(filename, lengthUnit, angleUnit, os());
    }

    virtual QWidget* getOptionPanelForSaving(BodyPositionItem* /* item */) override
    {
        return getOrCreateOptionPanel();
    }

    virtual void fetchOptionPanelForSaving() override
    {
        fetchOptionPanel();
    }

    virtual bool save(BodyPositionItem* item, const std::string& filename) override
    {
        return item->saveBodyPosition(filename, lengthUnit, angleUnit, os());
    }
};

}

void BodyPositionItem::initializeClass(cnoid::ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<BodyPositionItem>("BodyPositionItem")
        .addCreationPanel<BodyPositionItem>(new BodyPositionItemCreationPanel)
        .addFileIO<BodyPositionItem>(new BodyPositionItemFileIO);
}
