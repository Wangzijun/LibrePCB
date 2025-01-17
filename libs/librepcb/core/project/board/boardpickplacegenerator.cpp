/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "boardpickplacegenerator.h"

#include "../../export/pickplacedata.h"
#include "../../library/dev/device.h"
#include "../../library/pkg/package.h"
#include "../circuit/componentinstance.h"
#include "../project.h"
#include "board.h"
#include "items/bi_device.h"
#include "items/bi_footprintpad.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

BoardPickPlaceGenerator::BoardPickPlaceGenerator(const Board& board) noexcept
  : mBoard(board) {
}

BoardPickPlaceGenerator::~BoardPickPlaceGenerator() noexcept {
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

std::shared_ptr<PickPlaceData> BoardPickPlaceGenerator::generate() noexcept {
  // Assembly type map.
  static QMap<Package::AssemblyType, PickPlaceDataItem::Type> types = {
      {Package::AssemblyType::Tht, PickPlaceDataItem::Type::Tht},
      {Package::AssemblyType::Smt, PickPlaceDataItem::Type::Smt},
      {Package::AssemblyType::Mixed, PickPlaceDataItem::Type::Mixed},
      {Package::AssemblyType::Other, PickPlaceDataItem::Type::Other},
  };

  std::shared_ptr<PickPlaceData> data = std::make_shared<PickPlaceData>(
      *mBoard.getProject().getName(), mBoard.getProject().getVersion(),
      *mBoard.getName());
  const QStringList& locale = mBoard.getProject().getLocaleOrder();

  foreach (const BI_Device* device, mBoard.getDeviceInstances()) {
    QList<PickPlaceDataItem> items;
    const QString designator = *device->getComponentInstance().getName();
    const QString val = device->getComponentInstance().getValue(true).trimmed();
    const QString devName = *device->getLibDevice().getNames().value(locale);
    const QString pkgName = *device->getLibPackage().getNames().value(locale);

    // Determine fiducials to be exported.
    foreach (const BI_FootprintPad* pad, device->getPads()) {
      if (pad->getLibPad().getFunctionIsFiducial()) {
        QVector<PickPlaceDataItem::BoardSide> sides;
        if (pad->isOnLayer(Layer::topCopper())) {
          sides.append(PickPlaceDataItem::BoardSide::Top);
        }
        if (pad->isOnLayer(Layer::botCopper())) {
          sides.append(PickPlaceDataItem::BoardSide::Bottom);
        }
        const Angle rotation =
            pad->getMirrored() ? -pad->getRotation() : pad->getRotation();
        foreach (const auto side, sides) {
          items.append(PickPlaceDataItem(designator, val, devName, pkgName,
                                         pad->getPosition(), rotation, side,
                                         PickPlaceDataItem::Type::Fiducial));
        }
      }
    }

    // Ensure unique designators for pad items if there are multiple.
    if (items.count() > 1) {
      for (int i = 0; i < items.count(); ++i) {
        items[i].setDesignator(items[i].getDesignator() % ":" %
                               QString::number(i + 1));
      }
    }

    // Export device only if its package is something to mount.
    auto typeIt = types.find(device->getLibPackage().getAssemblyType(true));
    if (typeIt != types.end()) {
      const Point position = device->getPosition();
      const Angle rotation = device->getMirrored() ? -device->getRotation()
                                                   : device->getRotation();
      const PickPlaceDataItem::BoardSide boardSide = device->getMirrored()
          ? PickPlaceDataItem::BoardSide::Bottom
          : PickPlaceDataItem::BoardSide::Top;
      items.append(PickPlaceDataItem(designator, val, devName, pkgName,
                                     position, rotation, boardSide, *typeIt));
    }

    // Add all items.
    for (const PickPlaceDataItem& item : items) {
      data->addItem(item);
    }
  }

  return data;
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
