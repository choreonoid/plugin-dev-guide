#include "BodyPositionItem.h"
#include <cnoid/ItemManager>
#include <cnoid/ItemFileIO>
#include <cnoid/ValueTree>
#include <cnoid/EigenArchive>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <fmt/format.h>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>

using namespace std;
using namespace fmt;
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
    enum LengthUnit { Meter, Millimeter } lengthUnit;
    enum AngleUnit { Degree, Radian } angleUnit;
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
        lengthUnit = Meter;
        angleUnit = Degree;
    }

    virtual void storeOptions(Mapping* options) override
    {
        if(lengthUnit == Millimeter){
            options->write("length_unit", "millimeter");
        } else {
            options->write("length_unit", "meter");
        }
        if(angleUnit == Radian){
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
            lengthUnit = Millimeter;
        } else {
            lengthUnit = Meter;
        }
        options->read("angle_unit", unit);
        if(unit == "radian"){
            angleUnit = Radian;
        } else {
            angleUnit = Degree;
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
            lengthUnit = Meter;
        } else {
            lengthUnit = Millimeter;
        }
        if(angleUnitCombo->currentIndex() == 0){
            angleUnit = Degree;
        } else {
            angleUnit = Radian;
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
        YAMLReader reader;
        MappingPtr archive;
        try {
            archive = reader.loadDocument(filename)->toMapping();
        }
        catch(const ValueNode::Exception& ex){
            os() << ex.message() << endl;
        }
        double lengthRatio = 1.0;
        if(lengthUnit == Millimeter){
            lengthRatio /= 1000.0;
        }
        
        Isometry3 T = item->position();
        Vector3 v;
        if(read(archive, "translation", v)){
            T.translation() = lengthRatio * v;
        }
        if(read(archive, "rotation", v)){
            if(angleUnit == Degree){
                v = radian(v);
            }
            T.linear() = rotFromRpy(v);
        }
        item->setPosition(T);

        double height;
        if(archive->read("flag_height", height)){
            item->setFlagHeight(lengthRatio * height);
        }
        string color;
        if(archive->read("flag_color", color)){
            if(color == "red"){
                item->setFlagColor(BodyPositionItem::Red);
            } else if(color == "green"){
                item->setFlagColor(BodyPositionItem::Green);
            } else if(color == "blue"){
                item->setFlagColor(BodyPositionItem::Blue);
            }
        }
        return true;
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
        YAMLWriter writer;
        if(!writer.openFile(filename)){
            os() << format("Failed to open \"{0}\".", filename) << endl;
            return false;
        }
        
        MappingPtr archive = new Mapping;
        double lengthRatio = 1.0;
        if(lengthUnit == Millimeter){
            lengthRatio = 1000.0;
        }
        write(archive, "translation", Vector3(lengthRatio * item->position().translation()));
        Vector3 rpy = rpyFromRot(item->position().linear());
        if(angleUnit == Degree){
            rpy = degree(rpy);
        }
        write(archive, "rotation", rpy);
        archive->write("flag_height", lengthRatio * item->flagHeight());

        switch(static_cast<int>(item->flagColor())){
        case BodyPositionItem::Red:
            archive->write("flag_color", "red");
            break;
        case BodyPositionItem::Green:
            archive->write("flag_color", "green");
            break;
        case BodyPositionItem::Blue:
            archive->write("flag_color", "blue");
            break;
        }
        
        writer.putNode(archive);
        
        return true;
    }
};

}

void BodyPositionItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<BodyPositionItem>("BodyPositionItem")
        .addCreationPanel<BodyPositionItem>(new BodyPositionItemCreationPanel)
        .addFileIO<BodyPositionItem>(new BodyPositionItemFileIO);
}
