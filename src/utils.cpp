/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/08 10:38:29 by jolopez-          #+#    #+#             */
/*   Updated: 2025/04/08 11:00:23 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/utils.h"

int getClientFdByNickname(const std::map<int, std::string>& nicknames, const std::string& targetNickname)
{
    for (std::map<int, std::string>::const_iterator it = nicknames.begin(); it != nicknames.end(); ++it)
    {
        if (it->second == targetNickname)
        {
            return it->first;
        }
    }
    return -1;
}
