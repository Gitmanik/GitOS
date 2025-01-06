#pragma once

/**
 * @brief Default return
 * 
 */
#define ALL_OK 0

/**
 * @brief Encountered I/O Error
 * 
 */
#define EIO 1

/**
 * @brief Incorrent argument(s) provided
 * 
 */
#define EINVARG 2

/**
 * @brief Out of memory
 * 
 */
#define ENOMEM 3

/**
 * @brief Not implemented
 * 
 */
#define ENOTIMPL 4

/**
 * @brief Invalid format
 *
 */
#define EINFORMAT 5

/**
 * @brief Is taken
 * 
 */
#define EISTKN 8

#define ISERR(value) ((int) value < 0)
#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)