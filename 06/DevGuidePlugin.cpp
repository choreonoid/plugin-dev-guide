#include <cnoid/Plugin>
#include <cnoid/ItemManager>
#include <cnoid/Item>
#include <cnoid/RootItem>
#include <cnoid/BodyItem>
#include <cnoid/ItemList>
#include <cnoid/ToolBar>
#include <cnoid/RenderableItem>
#include <cnoid/SceneDrawables>
#include <cnoid/MeshGenerator>
#include <cnoid/EigenUtil>
#include <fmt/format.h>

using namespace std;
using namespace fmt;
using namespace cnoid;

class BodyPositionItem : public Item, public RenderableItem
{
    BodyItem* bodyItem;
    Isometry3 position;
    SgPosTransformPtr flag;
    
public:
    BodyPositionItem()
    {
        bodyItem = nullptr;
        position.setIdentity();
    }
    
    BodyPositionItem(const BodyPositionItem& org)
        : Item(org)
    {
        bodyItem = nullptr;
        position = org.position;
    }
    
    virtual Item* doDuplicate() const override
    {
        return new BodyPositionItem(*this);
    }

    virtual void onTreePathChanged() override
    {
        auto newBodyItem = findOwnerItem<BodyItem>();
        if(newBodyItem && newBodyItem != bodyItem){
            bodyItem = newBodyItem;
            mvout()
                << format("BodyPositionItem \"{0}\" has been attached to {1}.",
                          name(), bodyItem->name())
                << endl;
        }
    }

    void storeBodyPosition()
    {
        if(bodyItem){
            position = bodyItem->body()->rootLink()->position();
            updateFlagPosition();
            mvout()
                << format("The current position of {0} has been stored to {1}.",
                          bodyItem->name(), name())
                << endl;
        }
    }
            
    void restoreBodyPosition()
    {
        if(bodyItem){
            bodyItem->body()->rootLink()->position() = position;
            bodyItem->notifyKinematicStateChange(true);
            mvout()
                << format("The position of {0} has been restored from {1}.",
                          bodyItem->name(), name())
                << endl;
        }
    }

    virtual SgNode* getScene() override
    {
        if(!flag){
            flag = new SgPosTransform;
            MeshGenerator meshGenerator;
            constexpr double height = 1.8;

            auto pole = new SgShape;
            pole->setMesh(meshGenerator.generateCylinder(0.01, height));
            pole->getOrCreateMaterial()->setDiffuseColor(Vector3f(0.7f, 0.7f, 0.7f));
            auto polePos = new SgPosTransform;
            polePos->setRotation(AngleAxis(radian(90.0), Vector3::UnitX()));
            polePos->setTranslation(Vector3(0.0, 0.0, height / 2.0));
            polePos->addChild(pole);
            flag->addChild(polePos);

            auto ornament = new SgShape;
            ornament->setMesh(meshGenerator.generateSphere(0.02));
            ornament->getOrCreateMaterial()->setDiffuseColor(Vector3f(1.0f, 1.0f, 0.0f));
            auto ornamentPos = new SgPosTransform;
            ornamentPos->setTranslation(Vector3(0.0, 0.0, height + 0.01));
            ornamentPos->addChild(ornament);
            flag->addChild(ornamentPos);

            auto banner = new SgShape;
            banner->setMesh(meshGenerator.generateBox(Vector3(0.002, 0.3, 0.2)));
            banner->getOrCreateMaterial()->setDiffuseColor(Vector3f(1.0f, 1.0f, 1.0f));
            auto bannerPos = new SgPosTransform;
            bannerPos->setTranslation(Vector3(0.0, 0.16, height - 0.1));
            bannerPos->addChild(banner);
            flag->addChild(bannerPos);
        }

        updateFlagPosition();
        return flag;
    }

    void updateFlagPosition()
    {
        if(flag){
            auto p = position.translation();
            flag->setTranslation(Vector3(p.x(), p.y(), 0.0));
            auto rpy = rpyFromRot(position.linear());
            flag->setRotation(AngleAxis(rpy.z(), Vector3::UnitZ()));
            flag->notifyUpdate();
        }
    }
};

class DevGuidePlugin : public Plugin
{
public:
    DevGuidePlugin()
        : Plugin("DevGuide")
    {
        require("Body");
    }
        
    virtual bool initialize() override
    {
        itemManager()
            .registerClass<BodyPositionItem>("BodyPositionItem")
            .addCreationPanel<BodyPositionItem>();
        
        auto toolBar = new ToolBar("BodyPositionBar");
        toolBar->addButton("Store Body Positions")->sigClicked().connect(
            [this](){ storeBodyPositions(); });
        toolBar->addButton("Restore Body Positions")->sigClicked().connect(
            [this](){ restoreBodyPositions(); });
        toolBar->setVisibleByDefault();
        addToolBar(toolBar);

        return true;
    }
            
    void storeBodyPositions()
    {
        for(auto& item : RootItem::instance()->selectedItems<BodyPositionItem>()){
            item->storeBodyPosition();
        }
    }
    
    void restoreBodyPositions()
    {
        for(auto& item : RootItem::instance()->selectedItems<BodyPositionItem>()){
            item->restoreBodyPosition();
        }
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
