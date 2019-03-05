////////////////////////////////////////////////////////////////////////////////
// /****************************************************************************
// **
// ** Copyright (C) 2015-2019 M-Way Solutions GmbH
// ** Contact: https://www.blureange.io/licensing
// **
// ** This file is part of the Bluerange/FruityMesh implementation
// **
// ** $BR_BEGIN_LICENSE:GPL-EXCEPT$
// ** Commercial License Usage
// ** Licensees holding valid commercial Bluerange licenses may use this file in
// ** accordance with the commercial license agreement provided with the
// ** Software or, alternatively, in accordance with the terms contained in
// ** a written agreement between them and M-Way Solutions GmbH. 
// ** For licensing terms and conditions see https://www.bluerange.io/terms-conditions. For further
// ** information use the contact form at https://www.bluerange.io/contact.
// **
// ** GNU General Public License Usage
// ** Alternatively, this file may be used under the terms of the GNU
// ** General Public License version 3 as published by the Free Software
// ** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
// ** included in the packaging of this file. Please review the following
// ** information to ensure the GNU General Public License requirements will
// ** be met: https://www.gnu.org/licenses/gpl-3.0.html.
// **
// ** $BR_END_LICENSE$
// **
// ****************************************************************************/
////////////////////////////////////////////////////////////////////////////////

#include "GlobalState.h"

/**
 * The GlobalState was introduced to create multiple instances of FruityMesh
 * in a single process. This lets us do some simulation.
 */
GlobalState* GlobalState::instance;

#ifndef SIM_ENABLED
__attribute__ ((section (".noinit"))) RamRetainStruct ramRetainStruct;
__attribute__ ((section (".noinit"))) u32 rebootMagicNumber;
#else
RamRetainStruct ramRetainStruct;
u32 rebootMagicNumber;
#endif

GlobalState::GlobalState()
{
	//Some initialization
	currentEvent = (ble_evt_t *) currentEventBuffer;
	sizeOfCurrentEvent = 0;
	ramRetainStructPtr = &ramRetainStruct;
	rebootMagicNumberPtr = &rebootMagicNumber;
}


