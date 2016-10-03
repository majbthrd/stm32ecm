#ifndef __USBD_DESC_H
#define __USBD_DESC_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern const USBD_DescriptorsTypeDef VCP_Desc;
extern const uint8_t *USBD_CfgFSDesc_pnt;
extern const uint16_t USBD_CfgFSDesc_len;

#endif /* __USBD_DESC_H */
