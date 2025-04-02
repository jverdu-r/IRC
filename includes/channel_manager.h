/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_manager.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:38 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/02 19:39:19 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <map>
#include <set>
#include <string>

/*	La estructura Channel representa un canal de chat.
	-	name: nombre del canal.
	-	users: conjunto de usuarios en el canal.
	-	creator: usuario creador del canal.
*/
struct Channel
{
    std::string name;
    std::set<int> users;
    int creator;
};

#endif