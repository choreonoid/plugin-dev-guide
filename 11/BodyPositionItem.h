#ifndef DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_H
#define DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_H

#include <cnoid/Item>
#include <cnoid/RenderableItem>
#include <cnoid/BodyItem>
#include <cnoid/SceneGraph>
#include <cnoid/SceneDrawables>
#include <cnoid/Selection>

class BodyPositionItem : public cnoid::Item, public cnoid::RenderableItem
{
public:
    static void initializeClass(cnoid::ExtensionManager* ext);

    BodyPositionItem();
    BodyPositionItem(const BodyPositionItem& org);
    void setPosition(const cnoid::Isometry3& T);
    const cnoid::Isometry3& position() const { return position_; }
    void storeBodyPosition();
    void restoreBodyPosition();
    virtual cnoid::SgNode* getScene() override;
    bool setFlagHeight(double height);
    double flagHeight() const { return flagHeight_; }
    enum ColorId { Red, Green, Blue };
    bool setFlagColor(int colorId);
    double flagColor() const { return flagColorSelection.which(); }

    enum LengthUnit { Meter, Millimeter };
    enum AngleUnit { Degree, Radian };
    bool loadBodyPosition(
        const std::string& filename, LengthUnit lengthUnit, AngleUnit anguleUnit, std::ostream& os);
    bool saveBodyPosition(
        const std::string& filename, LengthUnit lengthUnit, AngleUnit anguleUnit, std::ostream& os);

    static cnoid::SignalProxy<void()> sigItemsInProjectChanged();

protected:
    virtual Item* doDuplicate() const override;
    virtual void onTreePathChanged() override;
    virtual void doPutProperties(cnoid::PutPropertyFunction& putProperty) override;
    virtual void notifyUpdate() override;
    virtual bool store(cnoid::Archive& archive) override;
    virtual bool restore(const cnoid::Archive& archive) override;
    virtual void onConnectedToRoot() override;
    virtual void onDisconnectedFromRoot() override;
    
private:
    void createFlag();
    void updateFlagPosition();
    void updateFlagMaterial();

    cnoid::BodyItem* bodyItem;
    cnoid::Isometry3 position_;
    cnoid::SgPosTransformPtr flag;
    double flagHeight_;
    cnoid::Selection flagColorSelection;
    cnoid::SgMaterialPtr flagMaterial;
};

typedef cnoid::ref_ptr<BodyPositionItem> BodyPositionItemPtr;

#endif // DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_H
