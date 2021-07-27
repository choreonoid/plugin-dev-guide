#include <cnoid/Plugin>
#include <cnoid/ConnectionSet>
#include <cnoid/ItemList>
#include <cnoid/RootItem>
#include <cnoid/BodyItem>
#include <cnoid/TimeBar>
#include <cnoid/EigenTypes>
#include <vector>

using namespace cnoid;

class DevGuidePlugin : public Plugin
{
    ScopedConnectionSet connections;
    ItemList<BodyItem> bodyItems;
    std::vector<Matrix3> initialRotations;
    
public:
    DevGuidePlugin() : Plugin("DevGuide")
    {
        require("Body");
    }
    
    virtual bool initialize() override
    {
        connections.add(
            RootItem::instance()->sigSelectedItemsChanged().connect(
                [this](const ItemList<>& selectedItems){
                    onSelectedItemsChanged(selectedItems);
                }));
                    
        connections.add(
            TimeBar::instance()->sigTimeChanged().connect(
                [this](double time){
                    return onTimeChanged(time);
                }));
        
        return true;
    }

    void onSelectedItemsChanged(ItemList<BodyItem> selectedBodyItems)
    {
        if(selectedBodyItems != bodyItems){
            bodyItems = selectedBodyItems;
            initialRotations.clear();
            for(auto& bodyItem : bodyItems){
                Body* body = bodyItem->body();
                Link* rootLink = body->rootLink();
                initialRotations.push_back(rootLink->rotation());
            }
        }
    }

    bool onTimeChanged(double time)
    {
        for(size_t i=0; i < bodyItems.size(); ++i){
            auto bodyItem = bodyItems[i];
            Matrix3 R = AngleAxis(time, Vector3::UnitZ()) * initialRotations[i];
            bodyItem->body()->rootLink()->setRotation(R);
            bodyItem->notifyKinematicStateChange(true);
        }

        return !bodyItems.empty();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
