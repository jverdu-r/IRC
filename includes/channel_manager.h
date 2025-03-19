/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_manager.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:38 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 16:41:54 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <map>
#include <set>
#include <string>

struct Channel {
    std::string name;
    std::set<int> users; // Lista de file descriptors de los usuarios en el canal
    int creator; // File descriptor del usuario que creó el canal
};

#endif