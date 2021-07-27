#include "BodyPositionItem.h"
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/EigenUtil>
#include <cnoid/PutPropertyFunction>
#include <fmt/format.h>

using namespace std;
using namespace fmt;
using namespace cnoid;

void BodyPositionItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<BodyPositionItem>("BodyPositionItem")
        .addCreationPanel<BodyPositionItem>();
}

BodyPositionItem::BodyPositionItem()
{
    bodyItem = nullptr;
    position_.setIdentity();
    flagColorSelection.setSymbol(Red, "red");
    flagColorSelection.setSymbol(Green, "green");
    flagColorSelection.setSymbol(Blue, "blue");
    flagColorSelection.select(Red);
    flagHeight_ = 1.8;
}
    
BodyPositionItem::BodyPositionItem(const BodyPositionItem& org)
    : Item(org)
{
    bodyItem = nullptr;
    position_ = org.position_;
    flagHeight_ = org.flagHeight_;
    flagColorSelection = org.flagColorSelection;
}
    
Item* BodyPositionItem::doDuplicate() const
{
    return new BodyPositionItem(*this);
}

void BodyPositionItem::onTreePathChanged()
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

void BodyPositionItem::storeBodyPosition()
{
    if(bodyItem){
        position_ = bodyItem->body()->rootLink()->position();
        updateFlagPosition();
        mvout()
            << format("The current position of {0} has been stored to {1}.",
                      bodyItem->name(), name())
            << endl;
    }
}

void BodyPositionItem::restoreBodyPosition()
{
    if(bodyItem){
        bodyItem->body()->rootLink()->position() = position_;
        bodyItem->notifyKinematicStateChange(true);
        mvout()
            << format("The position of {0} has been restored from {1}.",
                      bodyItem->name(), name())
            << endl;
    }
}

SgNode* BodyPositionItem::getScene()
{
    if(!flag){
        createFlag();
    }
    return flag;
}

void BodyPositionItem::createFlag()
{
    if(!flag){
        flag = new SgPosTransform;
        updateFlagPosition();
        flagMaterial = new SgMaterial;
        updateFlagMaterial();
    } else {
        flag->clearChildren();
    }
    
    MeshGenerator meshGenerator;
    
    auto pole = new SgShape;
    pole->setMesh(meshGenerator.generateCylinder(0.01, flagHeight_));
    pole->getOrCreateMaterial()->setDiffuseColor(Vector3f(0.7f, 0.7f, 0.7f));
    auto polePos = new SgPosTransform;
    polePos->setRotation(AngleAxis(radian(90.0), Vector3::UnitX()));
    polePos->setTranslation(Vector3(0.0, 0.0, flagHeight_ / 2.0));
    polePos->addChild(pole);
    flag->addChild(polePos);
    
    auto ornament = new SgShape;
    ornament->setMesh(meshGenerator.generateSphere(0.02));
    ornament->getOrCreateMaterial()->setDiffuseColor(Vector3f(1.0f, 1.0f, 0.0f));
    auto ornamentPos = new SgPosTransform;
    ornamentPos->setTranslation(Vector3(0.0, 0.0, flagHeight_ + 0.01));
    ornamentPos->addChild(ornament);
    flag->addChild(ornamentPos);
    
    auto banner = new SgShape;
    banner->setMesh(meshGenerator.generateBox(Vector3(0.002, 0.3, 0.2)));
    banner->setMaterial(flagMaterial);
    auto bannerPos = new SgPosTransform;
    bannerPos->setTranslation(Vector3(0.0, 0.16, flagHeight_ - 0.1));
    bannerPos->addChild(banner);
    flag->addChild(bannerPos);
}

void BodyPositionItem::updateFlagPosition()
{
    if(flag){
        auto p = position_.translation();
        flag->setTranslation(Vector3(p.x(), p.y(), 0.0));
        auto rpy = rpyFromRot(position_.linear());
        flag->setRotation(AngleAxis(rpy.z(), Vector3::UnitZ()));
        flag->notifyUpdate();
    }
}

void BodyPositionItem::updateFlagMaterial()
{
    if(flagMaterial){
        switch(flagColorSelection.which()){
        case Red:
            flagMaterial->setDiffuseColor(Vector3f(1.0f, 0.0f, 0.0f));
            break;
        case Green:
            flagMaterial->setDiffuseColor(Vector3f(0.0f, 1.0f, 0.0f));
            break;
        case Blue:
            flagMaterial->setDiffuseColor(Vector3f(0.0f, 0.0f, 1.0f));
            break;
        default:
            break;
        }
        flagMaterial->notifyUpdate();
    }
}        

void BodyPositionItem::setPosition(const Isometry3& T)
{
    position_ = T;
    updateFlagPosition();
    notifyUpdate();
}

bool BodyPositionItem::setFlagHeight(double height)
{
    if(height <= 0.0){
        return false;
    }
    flagHeight_ = height;
    if(flag){
        createFlag();
        flag->notifyUpdate();
    }
    notifyUpdate();
    return true;
}

bool BodyPositionItem::setFlagColor(int colorId)
{
    if(!flagColorSelection.select(colorId)){
        return false;
    }
    updateFlagMaterial();
    notifyUpdate();
    return true;
}

void BodyPositionItem::doPutProperties(PutPropertyFunction& putProperty)
{
    auto p = position_.translation();
    putProperty("Translation", format("{0:.3g} {1:.3g} {2:.3g}", p.x(), p.y(), p.z()),
                [this](const string& text){
                    Vector3 p;
                    if(toVector3(text, p)){
                        position_.translation() = p;
                        setPosition(position_);
                        return true;
                    }
                    return false;
                });

    auto r = degree(rpyFromRot(position_.linear()));
    putProperty("Rotation", format("{0:.0f} {1:.0f} {2:.0f}", r.x(), r.y(), r.z()),
                [this](const string& text){
                    Vector3 rpy;
                    if(toVector3(text, rpy)){
                        position_.linear() = rotFromRpy(radian(rpy));
                        setPosition(position_);
                        return true;
                    }
                    return false;
                });
    
    putProperty.min(0.1)("Flag height", flagHeight_,
                [this](double height){ return setFlagHeight(height); });

    putProperty("Flag color", flagColorSelection,
                [this](int which){ return setFlagColor(which); });
}
