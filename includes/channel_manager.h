/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_manager.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:38 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/02 23:11:31 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <set>
#include <string>

struct Channel
{
    int 			creator;
    std::string 	name;
	std::string 	topic;
    std::set<int> 	users;
	std::set<int> 	operators;
};
