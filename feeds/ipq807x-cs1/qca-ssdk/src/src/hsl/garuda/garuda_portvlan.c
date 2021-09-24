/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */



/**
 * @defgroup garuda_port_vlan GARUDA_PORT_VLAN
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "garuda_portvlan.h"
#include "garuda_reg.h"

#define MAX_VLAN_ID         4095


static sw_error_t
_garuda_port_1qmode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1qmode_t port_1qmode)
{
    sw_error_t rv;
    a_uint32_t regval[FAL_1Q_MODE_BUTT] = { 0, 3, 2, 1 };

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_1Q_MODE_BUTT <= port_1qmode)
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id, DOT1Q_MODE,
                      (a_uint8_t *) (&regval[port_1qmode]),
                      sizeof (a_uint32_t));

    return rv;

}


static sw_error_t
_garuda_port_1qmode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1qmode_t * pport_1qmode)
{
    sw_error_t rv;
    a_uint32_t regval = 0;
    fal_pt_1qmode_t retval[4] = { FAL_1Q_DISABLE, FAL_1Q_FALLBACK,
                                  FAL_1Q_CHECK, FAL_1Q_SECURE
                                };

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    SW_RTN_ON_NULL(pport_1qmode);

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id, DOT1Q_MODE,
                      (a_uint8_t *) (&regval), sizeof (a_uint32_t));

    SW_RTN_ON_ERROR(rv);

    *pport_1qmode = retval[regval & 0x3];

    return SW_OK;

}


static sw_error_t
_garuda_port_egvlanmode_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pt_1q_egmode_t port_egvlanmode)
{
    sw_error_t rv;
    a_uint32_t regval[FAL_EG_MODE_BUTT] = { 0, 1, 2, 3 };

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_EG_MODE_BUTT <= port_egvlanmode)
    {
        return SW_BAD_PARAM;
    }

    if (FAL_EG_HYBRID == port_egvlanmode)
    {
        return SW_NOT_SUPPORTED;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_CTL, port_id, EG_VLAN_MODE,
                      (a_uint8_t *) (&regval[port_egvlanmode]),
                      sizeof (a_uint32_t));

    return rv;

}


static sw_error_t
_garuda_port_egvlanmode_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pt_1q_egmode_t * pport_egvlanmode)
{
    sw_error_t rv;
    a_uint32_t regval = 0;
    fal_pt_1q_egmode_t retval[3] = { FAL_EG_UNMODIFIED, FAL_EG_UNTAGGED,
                                     FAL_EG_TAGGED
                                   };

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    SW_RTN_ON_NULL(pport_egvlanmode);

    HSL_REG_FIELD_GET(rv, dev_id, PORT_CTL, port_id, EG_VLAN_MODE,
                      (a_uint8_t *) (&regval), sizeof (a_uint32_t));

    SW_RTN_ON_ERROR(rv);

    *pport_egvlanmode = retval[regval & 0x3];

    return SW_OK;

}


static sw_error_t
_garuda_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id,
                            a_uint32_t mem_port_id)
{
    sw_error_t rv;
    a_uint32_t regval = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_FALSE == hsl_port_prop_check(dev_id, mem_port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) (&regval),
                      sizeof (a_uint32_t));

    SW_RTN_ON_ERROR(rv);

    regval |= (0x1UL << mem_port_id);

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) (&regval),
                      sizeof (a_uint32_t));

    return rv;

}

static sw_error_t
_garuda_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id,
                            a_uint32_t mem_port_id)
{
    sw_error_t rv;
    a_uint32_t regval = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_FALSE == hsl_port_prop_check(dev_id, mem_port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) (&regval),
                      sizeof (a_uint32_t));

    SW_RTN_ON_ERROR(rv);

    regval &= (~(0x1UL << mem_port_id));

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) (&regval),
                      sizeof (a_uint32_t));

    return rv;

}


static sw_error_t
_garuda_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id,
                               fal_pbmp_t mem_port_map)
{
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_FALSE ==
            hsl_mports_prop_check(dev_id, mem_port_map, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) (&mem_port_map),
                      sizeof (a_uint32_t));

    return rv;
}


static sw_error_t
_garuda_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pbmp_t * mem_port_map)
{
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_FALSE == hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    *mem_port_map = 0;
    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID_MEM, (a_uint8_t *) mem_port_map,
                      sizeof (a_uint32_t));

    SW_RTN_ON_ERROR(rv);

    return SW_OK;
}

static sw_error_t
_garuda_port_default_vid_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t vid)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if ((0 == vid) || (vid > MAX_VLAN_ID))
    {
        return SW_BAD_PARAM;
    }

    val = vid;
    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));

    return rv;
}


static sw_error_t
_garuda_port_default_vid_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t *vid)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      PORT_VID, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));

    *vid = val & 0xfff;
    return rv;
}

static sw_error_t
_garuda_port_force_default_vid_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      FORCE_DEF_VID, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_garuda_port_force_default_vid_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      FORCE_DEF_VID, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}


static sw_error_t
_garuda_port_force_portvlan_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      FORCE_PVLAN, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    return rv;
}


static sw_error_t
_garuda_port_force_portvlan_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_BASE_VLAN, port_id,
                      FORCE_PVLAN, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}

static sw_error_t
_garuda_port_nestvlan_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_EXCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_CTL, port_id,
                      DTAG_EN, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    return rv;
}


static sw_error_t
_garuda_port_nestvlan_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_EXCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_CTL, port_id,
                      DTAG_EN, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (1 == val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}


static sw_error_t
_garuda_nestvlan_tpid_set(a_uint32_t dev_id, a_uint32_t tpid)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    val = tpid;
    HSL_REG_FIELD_SET(rv, dev_id, SERVICE_TAG, 0,
                      TAG_VALUE, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    return rv;
}


static sw_error_t
_garuda_nestvlan_tpid_get(a_uint32_t dev_id, a_uint32_t *tpid)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    HSL_REG_FIELD_GET(rv, dev_id, SERVICE_TAG, 0,
                      TAG_VALUE, (a_uint8_t *) (&val),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    *tpid = val;
    return SW_OK;
}

/**
 * @brief Set 802.1q work mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] port_1qmode 802.1q work mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_1qmode_set(a_uint32_t dev_id, fal_port_t port_id,
                       fal_pt_1qmode_t port_1qmode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_1qmode_set(dev_id, port_id, port_1qmode);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get 802.1q work mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] port_1qmode 802.1q work mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_1qmode_get(a_uint32_t dev_id, fal_port_t port_id,
                       fal_pt_1qmode_t * pport_1qmode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_1qmode_get(dev_id, port_id, pport_1qmode);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set packets transmitted out vlan tagged mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] port_egvlanmode packets transmitted out vlan tagged mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_egvlanmode_set(a_uint32_t dev_id, fal_port_t port_id,
                           fal_pt_1q_egmode_t port_egvlanmode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_egvlanmode_set(dev_id, port_id, port_egvlanmode);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get packets transmitted out vlan tagged mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] port_egvlanmode packets transmitted out vlan tagged mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_egvlanmode_get(a_uint32_t dev_id, fal_port_t port_id,
                           fal_pt_1q_egmode_t * pport_egvlanmode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_egvlanmode_get(dev_id, port_id, pport_egvlanmode);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Add member of port based vlan on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] mem_port_id port member
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id,
                           a_uint32_t mem_port_id)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_portvlan_member_add(dev_id, port_id, mem_port_id);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Delete member of port based vlan on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] mem_port_id port member
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id,
                           a_uint32_t mem_port_id)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_portvlan_member_del(dev_id, port_id, mem_port_id);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Update member of port based vlan on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] mem_port_map port members
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id,
                              fal_pbmp_t mem_port_map)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_portvlan_member_update(dev_id, port_id, mem_port_map);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get member of port based vlan on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] mem_port_map port members
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id,
                           fal_pbmp_t * mem_port_map)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_portvlan_member_get(dev_id, port_id, mem_port_map);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set default vlan id on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] vid default vlan id
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_default_vid_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t vid)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_default_vid_set(dev_id, port_id, vid);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get default vlan id on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] vid default vlan id
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_default_vid_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t *vid)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_default_vid_get(dev_id, port_id, vid);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set force default vlan id status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_force_default_vid_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_force_default_vid_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get force default vlan id status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_force_default_vid_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_force_default_vid_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set force port based vlan status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_force_portvlan_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_force_portvlan_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get force port based vlan status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_force_portvlan_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_force_portvlan_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set nest vlan feature status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_nestvlan_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_nestvlan_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get nest vlan feature status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_port_nestvlan_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_port_nestvlan_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set nest vlan tpid on a particular device.
 * @param[in] dev_id device id
 * @param[in] tpid tag protocol identification
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_nestvlan_tpid_set(a_uint32_t dev_id, a_uint32_t tpid)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_nestvlan_tpid_set(dev_id, tpid);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get nest vlan tpid on a particular device.
 * @param[in] dev_id device id
 * @param[out] tpid tag protocol identification
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
garuda_nestvlan_tpid_get(a_uint32_t dev_id, a_uint32_t *tpid)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _garuda_nestvlan_tpid_get(dev_id, tpid);
    HSL_API_UNLOCK;
    return rv;
}

sw_error_t
garuda_portvlan_init(a_uint32_t dev_id)
{
    HSL_DEV_ID_CHECK(dev_id);

#ifndef HSL_STANDALONG
    hsl_api_t *p_api;

    SW_RTN_ON_NULL (p_api = hsl_api_ptr_get(dev_id));

    p_api->port_1qmode_get = garuda_port_1qmode_get;
    p_api->port_1qmode_set = garuda_port_1qmode_set;
    p_api->port_egvlanmode_get = garuda_port_egvlanmode_get;
    p_api->port_egvlanmode_set = garuda_port_egvlanmode_set;
    p_api->portvlan_member_add = garuda_portvlan_member_add;
    p_api->portvlan_member_del = garuda_portvlan_member_del;
    p_api->portvlan_member_update = garuda_portvlan_member_update;
    p_api->portvlan_member_get = garuda_portvlan_member_get;
    p_api->port_default_vid_set = garuda_port_default_vid_set;
    p_api->port_default_vid_get = garuda_port_default_vid_get;
    p_api->port_force_default_vid_set = garuda_port_force_default_vid_set;
    p_api->port_force_default_vid_get = garuda_port_force_default_vid_get;
    p_api->port_force_portvlan_set = garuda_port_force_portvlan_set;
    p_api->port_force_portvlan_get = garuda_port_force_portvlan_get;
    p_api->port_nestvlan_set = garuda_port_nestvlan_set;
    p_api->port_nestvlan_get = garuda_port_nestvlan_get;
    p_api->nestvlan_tpid_set = garuda_nestvlan_tpid_set;
    p_api->nestvlan_tpid_get = garuda_nestvlan_tpid_get;
#endif

    return SW_OK;
}

/**
 * @}
 */

