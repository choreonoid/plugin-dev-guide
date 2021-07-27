#include <cnoid/Plugin>
#include <cnoid/ItemList>
#include <cnoid/RootItem>
#include <cnoid/BodyItem>
#include <cnoid/ToolBar>
#include <cnoid/TimeBar>
#include <cnoid/DoubleSpinBox>
#include <cnoid/EigenTypes>
#include <vector>
#include <cmath>

using namespace cnoid;

class DevGuidePlugin : public Plugin
{
    ItemList<BodyItem> bodyItems;
    double initialTime;
    std::vector<Matrix3> initialRotations;
    DoubleSpinBox* speedRatioSpin;
    ToolButton* reverseToggle;
    
public:
    DevGuidePlugin() : Plugin("DevGuide")
    {
        require("Body");
    }
    
    virtual bool initialize() override
    {
        RootItem::instance()->sigSelectedItemsChanged().connect(
            [this](const ItemList<>& selectedItems){
                onSelectedItemsChanged(selectedItems);
            });
                    
        TimeBar::instance()->sigTimeChanged().connect(
            [this](double time){
                return onTimeChanged(time);
            });

        auto toolBar = new ToolBar("DevGuideBar");

        ToolButton* flipButton = toolBar->addButton("Flip");
        flipButton->sigClicked().connect(
            [this](){ flipBodyItems(); });

        reverseToggle = toolBar->addToggleButton("Reverse");
        reverseToggle->sigToggled().connect(
            [this](bool on){ updateInitialRotations(); });

        toolBar->addSeparator();

        toolBar->addLabel("Speed ratio");
        speedRatioSpin = new DoubleSpinBox;
        speedRatioSpin->setValue(1.0);
        speedRatioSpin->sigValueChanged().connect(
            [this](double value){ updateInitialRotations(); });
        toolBar->addWidget(speedRatioSpin);

        toolBar->setVisibleByDefault();
        addToolBar(toolBar);
        
        initialTime = 0.0;

        return true;
    }

    void onSelectedItemsChanged(ItemList<BodyItem> selectedBodyItems)
    {
        if(selectedBodyItems != bodyItems){
            bodyItems = selectedBodyItems;
            updateInitialRotations();
        }
    }

    void updateInitialRotations()
    {
        initialTime = TimeBar::instance()->time();
        initialRotations.clear();
        for(auto& bodyItem : bodyItems){
            initialRotations.push_back(bodyItem->body()->rootLink()->rotation());
        }
    }

    void flipBodyItems()
    {
        for(auto& bodyItem : bodyItems){
            Link* rootLink = bodyItem->body()->rootLink();
            Matrix3 R = AngleAxis(M_PI, Vector3::UnitZ()) * rootLink->rotation();
            rootLink->setRotation(R);
            bodyItem->notifyKinematicStateChange(true);
        }
        updateInitialRotations();
    }

    bool onTimeChanged(double time)
    {
        for(size_t i=0; i < bodyItems.size(); ++i){
            auto bodyItem = bodyItems[i];
            double angle = speedRatioSpin->value() * (time - initialTime);
            if(reverseToggle->isChecked()){
                angle = -angle;
            }
            Matrix3 R = AngleAxis(angle, Vector3::UnitZ()) * initialRotations[i];
            bodyItem->body()->rootLink()->setRotation(R);
            bodyItem->notifyKinematicStateChange(true);
        }

        return !bodyItems.empty();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
