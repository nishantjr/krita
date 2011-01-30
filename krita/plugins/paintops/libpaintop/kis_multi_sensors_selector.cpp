/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_multi_sensors_selector.h"

#include "ui_wdgmultisensorsselector.h"
#include "kis_multi_sensors_model_p.h"

struct KisMultiSensorsSelector::Private
{
  Ui_WdgMultiSensorsSelector form;
  KisMultiSensorsModel* model;
};

KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent) : d(new Private)
{
  d->form.setupUi(this);
  d->model = new KisMultiSensorsModel(this);
  connect(d->model, SIGNAL(sensorChanged(KisDynamicSensor*)), SIGNAL(sensorChanged(KisDynamicSensor*)));
  connect(d->model, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
  d->form.sensorsList->setModel(d->model);
}

KisMultiSensorsSelector::~KisMultiSensorsSelector()
{
  delete d;
}

void KisMultiSensorsSelector::setCurrent(KisDynamicSensor* _sensor)
{
  d->model->setCurrentSensor(_sensor);
}

KisDynamicSensor* KisMultiSensorsSelector::current()
{
  return 0;
}
