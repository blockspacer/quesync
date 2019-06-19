import React, { Component } from "react";
import { connect } from "react-redux";

import DrawerPage from "../components/page_layouts/drawer_page";
import DrawerItem from "../components/drawer_item";
import FriendRequestItem from "../components/friend_request_item";
import TextChannel from "../components/text_channel";

import {
	setFriendsPageSelectedTab,
	setFriendsPageSelectedDrawerItem
} from "../actions/itemsActions";

import "./friends_page.scss";

class FriendsPage extends Component {
	getPrivateChannelId = friendId => this.props.privateChannels[friendId];
	getSelectedFriendId = (friends, pendingFriends) => {
		// Friends selected
		if (this.props.selectedTab === 0) {
			return friends[this.props.selectedDrawerItem];
		} else {
			return pendingFriends[this.props.selectedDrawerItem].id;
		}
	};

	render() {
		const friends = this.props.user.friends
			? this.props.user.friends.map(friendId => ({
					id: friendId,
					nickname: this.props.profiles[friendId].nickname
			  }))
			: [];

		const pendingFriends = this.props.user.friendRequests
			? this.props.user.friendRequests
					.map(({ friendId, sentAt }) => ({
						id: friendId,
						nickname: this.props.profiles[friendId].nickname,
						sentAt
					}))
					.sort((a, b) => b.sentAt - a.sentAt)
			: [];

		const currentSelectedFriendId =
			this.props.selectedDrawerItem !== -1
				? this.getSelectedFriendId(friends, pendingFriends)
				: null;

		return (
			<DrawerPage
				className="quesync-friends-page"
				drawerTabs={["All", "Pending"]}
				selectedDrawerTab={this.props.selectedTab}
				drawerTabSelected={tabIdx => {
					this.props.dispatch(setFriendsPageSelectedTab(tabIdx));

					// Reset the choice
					this.props.dispatch(setFriendsPageSelectedDrawerItem(-1));
				}}
				drawerContent={[
					friends.map((friend, idx) => (
						<DrawerItem
							key={friend.id}
							avatarUrl="https://jamesmfriedman.github.io/rmwc/images/avatars/captainamerica.png"
							itemName={friend.nickname}
						/>
					)),
					pendingFriends.map((friend, idx) => (
						<FriendRequestItem
							key={friend.id}
							friendAvatarUrl="https://jamesmfriedman.github.io/rmwc/images/avatars/captainamerica.png"
							friendName={friend.nickname}
							sentAt={friend.sentAt}
						/>
					))
				]}
				tableWidth="14rem"
				drawerItemClicked={friendIdx =>
					this.props.dispatch(setFriendsPageSelectedDrawerItem(friendIdx))
				}
				badgeBGColor="red"
				badgeColor="white"
				drawerTabsBadges={[
					this.props.allFriendsBadge,
					this.props.pendingFriendsBadge
				]}>
				{this.props.selectedDrawerItem !== -1 ? (
					<TextChannel
						channelId={this.getPrivateChannelId(currentSelectedFriendId)}
					/>
				) : null}
			</DrawerPage>
		);
	}
}

export default connect(state => ({
	user: state.auth.user,
	profiles: state.users.profiles,
	selectedTab: state.ui.items.selectedFriendsPageTab,
	selectedDrawerItem: state.ui.items.selectedFriendsPageDrawerItem,
	allFriendsBadge: state.ui.badges.allFriendsBadge,
	pendingFriendsBadge: state.ui.badges.pendingFriendsBadge,
	privateChannels: state.channels.privateChannels
}))(FriendsPage);
