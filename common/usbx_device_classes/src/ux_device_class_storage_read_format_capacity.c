/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** USBX Component                                                        */ 
/**                                                                       */
/**   Device Storage Class                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_device_class_storage.h"
#include "ux_device_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_device_class_storage_read_format_capacity       PORTABLE C      */ 
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function performs a READ_FORMAT_CAPACITY command.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    storage                               Pointer to storage class      */ 
/*    endpoint_in                           Pointer to IN endpoint        */
/*    endpoint_out                          Pointer to OUT endpoint       */
/*    cbwcb                                 Pointer to CBWCB              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_device_class_storage_csw_send     Send CSW                      */ 
/*    _ux_device_stack_transfer_request     Transfer request              */ 
/*    _ux_utility_memory_set                Set memory                    */
/*    _ux_utility_long_put_big_endian       Put 32-bit big endian         */
/*    _ux_utility_memory_copy               Copy memory                   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Device Storage Class                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_device_class_storage_read_format_capacity(UX_SLAVE_CLASS_STORAGE *storage, ULONG lun,
                                            UX_SLAVE_ENDPOINT *endpoint_in,
                                            UX_SLAVE_ENDPOINT *endpoint_out, UCHAR * cbwcb)
{

UINT                    status;
UX_SLAVE_TRANSFER       *transfer_request;
UCHAR                   read_format_capacity_buffer[UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LENGTH];

    UX_PARAMETER_NOT_USED(cbwcb);
    UX_PARAMETER_NOT_USED(endpoint_out);

    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_DEVICE_CLASS_STORAGE_READ_FORMAT_CAPACITY, storage, lun, 0, 0, UX_TRACE_DEVICE_CLASS_EVENTS, 0, 0)

    /* Obtain the pointer to the transfer request.  */
    transfer_request =  &endpoint_in -> ux_slave_endpoint_transfer_request;

    /* Ensure it is cleaned.  */
    _ux_utility_memory_set(read_format_capacity_buffer, 0, UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LENGTH);

    /* Insert the size of the response block.  */
    _ux_utility_long_put_big_endian(&read_format_capacity_buffer[UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_SIZE], 8);

    /* Insert the last LBA address in the response.  */
    _ux_utility_long_put_big_endian(&read_format_capacity_buffer[UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LAST_LBA],
                                    storage -> ux_slave_class_storage_lun[lun].ux_slave_class_storage_media_last_lba);

    /* Insert the block length in the response.  This is in 3 bytes. */
    _ux_utility_long_put_big_endian(&read_format_capacity_buffer[UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_BLOCK_SIZE],
                                    storage -> ux_slave_class_storage_lun[lun].ux_slave_class_storage_media_block_length);

    /* Insert the response code : always 2.  */
    read_format_capacity_buffer[UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_DESC_CODE] =  2;    

    /* Copy the CSW into the transfer request memory.  */
    _ux_utility_memory_copy(transfer_request -> ux_slave_transfer_request_data_pointer, 
                                        read_format_capacity_buffer, UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LENGTH);

    /* Send a data payload with the read_capacity response buffer.  */
    _ux_device_stack_transfer_request(transfer_request, 
                                  UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LENGTH,
                                  UX_SLAVE_CLASS_STORAGE_READ_FORMAT_CAPACITY_RESPONSE_LENGTH);

    /* Now we return a CSW with success.  */
    status =  _ux_device_class_storage_csw_send(storage, lun, endpoint_in, UX_SLAVE_CLASS_STORAGE_CSW_PASSED);

    /* Return completion status.  */
    return(status);
}

