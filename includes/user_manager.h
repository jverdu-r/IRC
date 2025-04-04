/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_manager.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:24 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/02 23:11:43 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include <string>
#include <map>
#include <set>

class UserManager
{
	public:
		UserManager(std::map<int, std::string>& usernames);
		~UserManager();
		
		void 					setUserName(int client_fd, const std::string& username);
		std::string				getUserName(int client_fd);
		bool					userNameExists(const std::string& username);

		void 					addUserChannel(int client_fd, const std::string& channel);
		void 					removeUserChannel(int client_fd, const std::string& channel);
		std::set<std::string>	getUserChannels(int client_fd) const;

		std::set<std::string>	findChannelsByUsername(const std::string& username) const;
		std::set<std::string>	findChannelsByNickname(const std::string& nickname, const std::map<int, std::string>& nicknames) const;

	private:
		std::map<int, std::string>&				usernames;
		std::map<int, std::set<std::string> >	user_channels;

};
