/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_manager.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:24 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 17:52:28 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <map>

class UserManager
{
	public:
		UserManager(std::map<int, std::string>& usernames);
		void setUserName(int client_fd, const std::string& username);
		std::string getUserName(int client_fd);
		std::string getUserChannel(int client_fd) const;
		void setUserChannel(int client_fd, const std::string& channel);
	private:
		std::map<int, std::string>& usernames;
		std::map<int, std::string> user_channels;
};

#endif